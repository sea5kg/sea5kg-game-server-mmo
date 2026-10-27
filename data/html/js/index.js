
document.addEventListener("keydown", event => {
  // console.log(event.keyCode);
  if (event.keyCode == 38) {
    // userMoveUp();
  } else if (event.keyCode == 40) {
    // userMoveDown();
  } else if (event.keyCode == 37) {
    // userMoveLeft()
  } else if (event.keyCode == 39) {
    // userMoveRight()
  }
});

window.gs_mmo_api_interval = null;
window.gs_mmo_api_reconnect_countdown = 0;
window.gs_mmo_api = new GameServerMMOApi();

function load_first_map(player_coordinates) {

  // gs_mmo_api.ws_get_map(player_coordinates).done(function(result) {
  //     var objects = result["objects"];
  //     for (var i in objects) {
  //         var obj = objects[i];
  //         if (obj.l == VVAPI_LAYER_BACKGROUND) {
  //             gs_mmo_render.add_background_object(obj)
  //         } else if (obj.l == VVAPI_LAYER_ROADS) {
  //             gs_mmo_render.add_road_object(obj)
  //         } else if (obj.l == VVAPI_LAYER_VEGETATION) {
  //             gs_mmo_render.add_vegetation_object(obj)
  //         } else if (obj.l == VVAPI_LAYER_BUILDING) {
  //             gs_mmo_render.add_building_object(obj)
  //         } else {
  //             console.error("Unknown layer")
  //         }
  //     }
  //     console.log("ws_get_map = ", result);
  //     gs_mmo_render.resized();
  //     gs_mmo_render.update();
  // });
  gs_mmo_render.resized();
  gs_mmo_render.update();
}

function getRandomInt(min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

document.addEventListener('DOMContentLoaded', function () {
  console.log("Content loaded");

  resize_canvas();
  window.gs_mmo_render = new GameServerMMORender('game_window_render');

  // window.gs_mmo_api.ws_get_player_position().done(function (data) {
  //   console.log("ws_get_player_position: ", data);
  //   gs_mmo_render.player_coordinates = data;
  //   gs_mmo_render.player_target_coordinates = data;
  //   load_first_map(gs_mmo_render.player_coordinates)
  // })

  window.gs_mmo_api.on_ws_state((state) => {
    console.log("on_ws_state ", state);
    if (!state) {
      return;
    }
    const el = document.getElementById("ws_status");
    const el_text = document.getElementById("ws_status_text");
    el.classList = "ws-status";
    switch (state) {
      case "connecting":
        el.classList.add("connecting");
        el_text.textContent = "Connecting...";
        break;
      case "connected":
        el_text.textContent = "connected";
        el.classList.add("connected");
        break;
      case "disconnected":
        window.gs_mmo_api_reconnect_countdown = getRandomInt(2, 15);
        el_text.innerHTML = "Disconnected.<br/><br/><small>Reconnect after " + window.gs_mmo_api_reconnect_countdown + " sec</small>";
        el.classList.add("disconnected");
        if (window.gs_mmo_api_interval) {
          clearInterval(window.gs_mmo_api_interval);
        }
        window.gs_mmo_api_interval = setInterval(function () {
          if (window.gs_mmo_api_reconnect_countdown > 0) {
            window.gs_mmo_api_reconnect_countdown--;
            el_text.innerHTML = "Disconnected.<br/><br/><small>Reconnect after " + window.gs_mmo_api_reconnect_countdown + " sec</small>";
            if (window.gs_mmo_api_reconnect_countdown > 0)
              return;
          }
          if (window.gs_mmo_api_reconnect_countdown == 0) {
            window.gs_mmo_api.reconnect_1500();
            clearInterval(window.gs_mmo_api_interval);
          }
        }, 1000);
        break;
    }
  });

  window.gs_mmo_api.on_init_player(window.gs_mmo_render.on_init_player);
  window.gs_mmo_api.on_init_players(window.gs_mmo_render.on_init_players);
  window.gs_mmo_api.on_players_events(window.gs_mmo_render.on_players_events);
});

function resize_canvas() {

  // console.log(window.innerWidth);
  var el = document.getElementById('game_window_render');
  el.style['width'] = window.innerWidth + 'px';
  el.style['height'] = window.innerHeight + 'px';
  el.width = window.innerWidth;
  el.height = window.innerHeight;

  if (window.gs_mmo_render) {
    window.gs_mmo_render.resized();
    window.gs_mmo_render.update();
  }
}

window.addEventListener("resize", resize_canvas);

setInterval(() => {
  if (window.gs_mmo_render) {
    window.gs_mmo_render.update();
  }
}, 40);