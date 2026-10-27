
// TODO get variables from server
VVAPI_LAYER_BACKGROUND = 1;
VVAPI_LAYER_ROADS = 2;
VVAPI_LAYER_VEGETATION = 3;
VVAPI_LAYER_BUILDING = 4;

class GameServerMMOApiEventListener {
  constructor(init_data) {
    this.__listeners = [];
    var self = this;
    self.__last_data = init_data;
  };

  set_data(data) {
    this.__last_data = data;
    this.__listeners.forEach(fn => fn(data));
  }

  get_last_data() {
    return this.__last_data;
  }

  add(fn) {
    this.__listeners.push(fn);
    fn(self.__last_data); // fire immediately with current state
    return () => {
      this.__listeners = this.__listeners.filter(f => f !== fn);
    };
  }
};


class GameServerMMOApi {
  constructor() {
    this.base_url = location.protocol + "//" + location.host;
    this.ws_req_id = 0;
    this.ws_requests = {};
    this.__ws_state = new GameServerMMOApiEventListener(""); // "connecting" | "connected" | "disconnected"
    // this.__init_player = new GameServerMMOApiEventListener({"id": "", "x": 0, "y": 0});
    this.__init_player = new GameServerMMOApiEventListener();
    this.__init_players = new GameServerMMOApiEventListener();
    this.__players_events = new GameServerMMOApiEventListener();
    this.reconnect_1500();
  }

  reconnect_1500() {
    var self = this;
    this._set_ws_state("connecting");
    setTimeout(function() {
      self.connect();
    }, 1500);
  }

  connect() {
    if (!"WebSocket" in window) {
      this.error = "Not found WebSocket!";
      self._set_ws_state("disconnected");
      return false;
    }

    var self = this;
    this._set_ws_state("connecting");
    this.ws = new WebSocket("ws://" + location.host + "/ws-api/");

    this.ws.onopen = function () {
      console.log("open");
      self._set_ws_state("connected");
      // still "connecting" until the auth response comes back
      var auth_params = {};
      if (sessionStorage.getItem("my-secret")) {
        auth_params["secret"] = sessionStorage.getItem("my-secret");
      }
      self.ws.send(JSON.stringify({
        "id": "auth",
        "method": "auth",
        "params": auth_params
      }));

      // self.ws.send(JSON.stringify({ "method": "hello" }));
      // if (!localStorage.getItem("last-room")) {
      //   self.ws.send(JSON.stringify({ "method": "goto_room" }));
      // }
    };

    this.ws.onmessage = function (ev) {
      var received_msg = ev.data;
      // console.log("received_msg:", received_msg);
      var resp = JSON.parse(received_msg);
      var msg_id = resp["id"];

      if (msg_id == "auth") {
        if (resp["result"]["secret"] !== undefined) {
          sessionStorage.setItem("my-secret", resp["result"]["secret"]);
          delete resp["result"]["secret"];
        }
        self.__init_player.set_data(resp["result"])
        self._set_ws_state("connected");
      }

      if (self.ws_requests[msg_id]) {
        self.ws_requests[msg_id].resolve(resp["result"]);
        delete self.ws_requests[msg_id];
      }
      if (resp["method"] !== undefined) {
        if (resp["method"] === "init_players") {
          self.__init_players.set_data(resp["params"]);
        } else if (resp["method"] === "players_events") {
          self.__players_events.set_data(resp["params"]);
        } else {
          console.warn("No handled method", resp["method"]);
        }
      }
      console.log("received websocket message: " + received_msg);
    };

    this.ws.onclose = function () {
      console.log("closed");
      self._set_ws_state("disconnected");
    };

    this.ws.onerror = function () {
      console.log("ws-error");
      self._set_ws_state("disconnected");
    };
  }

  _set_ws_state(state) {
    if (this.__ws_state.get_last_data() === state) return;
    this.__ws_state.set_data(state);
  }

  on_ws_state(fn) {
    return this.__ws_state.add(fn);
  }

  on_init_player(fn) {
    return this.__init_player.add(fn);
  }

  on_init_players(fn) {
    return this.__init_players.add(fn);
  }

  on_players_events(fn) {
    return this.__players_events.add(fn);
  }

  promise() {
    var d = {};
    d.completed = false;
    d.failed = false;
    d.successed = false;
    d.done = function (callback) {
      d.done_callback = callback;
      if (d.completed && typeof d.done_callback === "function" && d.successed) {
        d.done_callback.apply(this, d.result_arguments);
      }
      return d;
    }

    d.fail = function (callback) {
      d.fail_callback = callback;
      if (d.completed && typeof d.fail_callback === "function" && d.failed) {
        d.fail_callback.apply(this, d.error_arguments);
      }
      return d;
    }

    d.resolve = function () {
      if (!d.completed) {
        d.result_arguments = arguments; // [];
        if (typeof d.done_callback === "function") {
          d.done_callback.apply(this, d.result_arguments);
        }
      }
      d.successed = true;
      d.completed = true;
    }
    d.reject = function () {
      if (!d.completed) {
        d.error_arguments = arguments;
        if (typeof d.fail_callback === "function") {
          d.fail_callback.apply(this, d.error_arguments);
        }
      }
      d.failed = true;
      d.completed = true;
    }
    return d;
  };

  waitPromises(arr_promise) {
    var p = this.promise();
    var max_len = arr_promise.length;
    var result = [];
    function cmpl(r) {
      result.push(r);
      if (result.length == max_len) {
        p.resolve(result);
      }
    };
    for (var i in arr_promise) {
      arr_promise[i].done(cmpl).fail(cmpl);
    }
    return p;
  }

  request(path, d) {
    var xhr = new XMLHttpRequest()
    xhr.onload = function () {
      // alert(`Загружено: ${xhr.status} ${xhr.response}`);
    };
    xhr.onerror = function () {
      console.error('Ошибка соединения');
      d.reject("error");
    };
    xhr.onreadystatechange = function () {
      if (xhr.readyState != 4) {
        return
      }
      if (xhr.status === 200) {
        d.resolve(JSON.parse(xhr.responseText));
      } else {
        d.reject("error: " + xhr.responseText);
      }
    }
    xhr.open(
      'GET',
      this.base_url + path,
      true
    );
    xhr.send();
  }

  load_map() {
    var d = this.promise();
    this.request("/api/v1/get_map?v=1", d);
    return d;
  }

  ws_request(methodname, params, d) {
    var defer = d || this.promise();
    var _params = params || {};
    if (this.__ws_state.get_last_data() !== "connected") {
       console.error("No connection.");
      // console.warn("No connection. retry after 100ms");
      // var self = this;
      // setTimeout(function () {
      //   self.ws_request(methodname, _params, defer);
      // }, 100);
    } else {
      console.log(methodname, params);
      var new_id = this.ws_req_id++;
      this.ws.send(JSON.stringify({
        "id": new_id,
        "method": methodname,
        "params": params
      }))
      this.ws_requests[new_id] = defer;
    }
    return defer;
  }

  ws_change_player_position(x, y) {
    return this.ws_request("change_player_position", { "x": parseInt(x, 10), "y": parseInt(y, 10) });
  }

  ws_get_player_position() {
    var defer = this.promise();
    defer.resolve(this.__init_player.get_last_data());
    return defer;
    // console.log("get_player_position");
    // return this.ws_request("get_player_position", {});
  }

  ws_destroy_road(road) {
    console.log("ws_destroy_road");
    return this.ws_request("ws_destroy_road", road);
  }

  ws_get_map(data) {
    return this.ws_request("get_map", data);
  }
};
