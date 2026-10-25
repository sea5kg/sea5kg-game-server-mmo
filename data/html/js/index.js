
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
    const el = document.getElementById("ws-status");
    switch (state) {
      case "connecting":
        el.textContent = "Connecting…";
        el.className = "ws-status connecting";
        break;
      case "connected":
        el.textContent = "Connected";
        el.className = "ws-status connected";
        break;
      case "disconnected":
        el.textContent = "Disconnected";
        el.className = "ws-status disconnected";
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