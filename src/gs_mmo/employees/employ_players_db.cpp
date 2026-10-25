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

#include <wsjcpp_employees.h>
#include <wsjcpp_yaml.h>
#include <wsjcpp_core.h>
#include <sstream>
#include <thread>
#include <cstring>
#include <ctime>
#include <locale>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sea5kg_logger.h>
#include <sea5kg_sqlite3_wrapper.h>
#include "gs_mmo/include/gs_mmo_globals.h"
#include "gs_mmo/include/gs_mmo_config.h"
#include "gs_mmo/include/gs_mmo_players_db.h"
#include <sys/stat.h>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

class employ_players_db : public WsjcppEmployBase, public gs_mmo::players_db {
public:
  employ_players_db();
  ~employ_players_db();
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // gs_mmo::employ_players_db

private:
  bool init_players_db();

  std::string TAG;
  std::mutex m_mutex_players_db;
  std::shared_ptr<sea5kg::sqlite3_wrapper::database_file> m_players_db;
};

REGISTRY_WSJCPP_EMPLOY(employ_players_db)

employ_players_db::employ_players_db()
: WsjcppEmployBase({ gs_mmo::players_db::name() }, { gs_mmo::config::name() }) {
  TAG = gs_mmo::players_db::name();
}

employ_players_db::~employ_players_db() {
}

bool employ_players_db::init(const std::string &sName, bool bSilent) {
  if (!init_players_db()) {
    return false;
  }
  return true;
}

bool employ_players_db::deinit(const std::string &sName, bool bSilent) {
  sea5kg::log::info(TAG, "deinit");
  return true;
}

CLASS_DATABASE_UPDATE_BEGIN(database_players, initial, v001, "Init table players") {
  // IF NOT EXISTS
  return db->execute_query(
    "CREATE TABLE IF NOT EXISTS players ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  player_id VARCHAR(50) NOT NULL, "
    "  secret VARCHAR(50) NOT NULL, "
    "  created_dt INTEGER NOT NULL, "
    "  updated_dt INTEGER NOT NULL, "
    "  pos_x INTEGER NOT NULL, " // TODO
    "  pos_y INTEGER NOT NULL "
    ");",
    error
  );
}
CLASS_DATABASE_UPDATE_END()

bool employ_players_db::init_players_db() {
  std::lock_guard<std::mutex> lock(m_mutex_players_db);
  m_players_db = std::make_shared<sea5kg::sqlite3_wrapper::database_file>(
    "database_players",
    // TODO result of send_flag (error code or success + elapsed time)
    findWsjcppEmploy<gs_mmo::config>()->db_dir(),
    "players.db",
    gs_mmo::DEFAULT_DATABASE_BACKUP_FREQUENCY_IN_SECONDS
  );
  std::string error;
  if (m_players_db->has_error(error)) {
    sea5kg::log::critical(TAG, error);
    return false;
  }
  sea5kg::log::info(TAG, "Opening players.db");
  if (!m_players_db->open(error)) {
    sea5kg::log::critical(TAG, "Problem with open database. Error: " + error);
    return false;
  }
  return true;
}