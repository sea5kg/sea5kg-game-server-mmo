/**********************************************************************************
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * ::                                   oooooo 8                                 ::
 * ::                                   8      8                                 ::
 * ::              .oPYo. .oPYo. .oPYo. 8pPYo. 8  .o  .oPYo.                     ::
 * ::              Yb..   8oooo8 .oooo8     `8 8oP'   8    8                     ::
 * ::                'Yb. 8.     8    8     .P 8 `b.  8    8                     ::
 * ::              `YooP' `Yooo' `YooP8 `YooP' 8  `o. `YooP8                     ::
 * :::::::::::::::::.....::.....::.....::.....:..::...:....8 ::::::::::::::::::::::
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::ooP'.::::::::::::::::::::::
 * :: .oPYo. .oPYo. ooYoYo. .oPYo.       .oPYo. .oPYo. oPYo. o    o .oPYo. oPYo. ::
 * :: 8    8 .oooo8 8' 8  8 8oooo8 ooooo Yb..   8oooo8 8  `' Y.  .P 8oooo8 8  `' ::
 * :: 8    8 8    8 8  8  8 8.             'Yb. 8.     8     `b..d' 8.     8     ::
 * :: `YooP8 `YooP8 8  8  8 `Yooo'       `YooP' `Yooo' 8      `YP'  `Yooo' 8     ::
 * ::::....8 :.....:..:..:..:.....::::::::.....::.....:..::::::...:::.....:..::::::
 * :::::ooP'.:::::::ooYoYo. ooYoYo. .oPYo.                                       ::
 * ::               8' 8  8 8' 8  8 8    8                                       ::
 * ::               8  8  8 8  8  8 8    8                                       ::
 * ::               8  8  8 8  8  8 `YooP'                                       ::
 * :::::::::::::::::..:..:....:..:..:.....:::::::::::::::::::::::::::::::::::::::::
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 * MIT License
 *
 * Copyright (c) 2020-2026 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Original repository: https://github.com/sea5kg/sea5kg-game-server-mmo
 *
 ***********************************************************************************/

#pragma once

#include <string>
#include <mutex>
#include "gs_mmo_player_context.h"
#include "third_party/nlohmann/json.hpp"

namespace gs_mmo {

enum class game_net_events_type : int {
  ADD_PLAYER = 1,
  REMOVE_PLAYER = 2,
  MOVE_PLAYER = 3
};

class game_net_events {
public:
  game_net_events();
  void set_method_name(const std::string &method);
  void set_collect_events(bool v);
  void add_player(const gs_mmo::player_context &ctx);
  void remove_player(const std::string &player_name);
  void update_player(const gs_mmo::player_context &ctx);
  // void add_event(const nlohmann::json &);
  std::string to_string_json_request();
  long get_last_time_event_ms() const;
  std::string pop_and_reset(long &last_update);

private:
  void unsafe_add_event(const nlohmann::json &);
  void unsafe_reset();
  std::mutex m_mutex;
  bool m_collect_events;
  nlohmann::json m_resp;
  long m_last_events_ms;
};

} // namespace gs_mmo
