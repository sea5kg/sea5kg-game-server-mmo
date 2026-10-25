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

#include "gs_mmo/include/gs_mmo_config.h"
#include "gs_mmo/include/gs_mmo_globals.h"
#include "gs_mmo/include/gs_mmo_players_db.h"
#include "gs_mmo/utils/gs_mmo_time_measurer.h"
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <sea5kg_logger.h>
#include <sea5kg_sqlite3_wrapper.h>
#include <sstream>
#include <stdio.h>
#include <sys/stat.h>
#include <thread>
#include <wsjcpp_core.h>
#include <wsjcpp_employees.h>
#include <wsjcpp_yaml.h>

class employ_players_db : public WsjcppEmployBase, public gs_mmo::players_db {
public:
  employ_players_db();
  ~employ_players_db();
  virtual bool init(const std::string &name, bool silent) override;
  virtual bool deinit(const std::string &name, bool silent) override;

  // gs_mmo::employ_players_db
  virtual std::shared_ptr<gs_mmo::player_db_info> create_player() override;
  virtual std::shared_ptr<gs_mmo::player_db_info> get_player_by_secret(const std::string &secret) override;
  virtual void update_last_position(int id, int x, int y, bool force_update = false) override;

private:
  bool init_players_db();
  std::string generate_random_secret();

  std::string TAG;
  std::mutex m_mutex_players_db;
  std::shared_ptr<sea5kg::sqlite3_wrapper::database_file> m_players_db;
  std::map<long, std::shared_ptr<gs_mmo::player_db_info>> m_cache_id_players;
  std::map<std::string, std::shared_ptr<gs_mmo::player_db_info>> m_cache_secret_players;
};

REGISTRY_WSJCPP_EMPLOY(employ_players_db)

employ_players_db::employ_players_db() : WsjcppEmployBase({gs_mmo::players_db::name()}, {gs_mmo::config::name()}) {
  TAG = gs_mmo::players_db::name();
}

employ_players_db::~employ_players_db() {
}

bool employ_players_db::init(const std::string &sName, bool bSilent) {
  if (!init_players_db()) {
    return false;
  }

  {
    gs_mmo::time_measurer measurer("restore players from database");
    auto config = findWsjcppEmploy<gs_mmo::config>();
    std::lock_guard<std::mutex> lock(m_mutex_players_db);
    // TODO cleanup old inactive records
    std::string error;
    auto it = m_players_db->select_rows(
      "SELECT id, player_name, secret, created_dt, updated_dt, pos_x, pos_y FROM players", error
    );
    if (it == nullptr) {
      sea5kg::log::critical(TAG, "Unexpected error for select, Error: " + error);
      return false;
    }
    while (it->next()) {
      auto row = std::make_shared<gs_mmo::player_db_info>();
      row->id = it->as_long(0);
      row->player_name = it->as_string(1);
      row->secret = it->as_string(2);
      row->created_dt = it->as_long(3);
      row->updated_dt = it->as_long(4);
      row->pos_x = it->as_long(5);
      row->pos_y = it->as_long(6);
      m_cache_id_players[row->id] = row;
      m_cache_secret_players[row->secret] = row;
    }
  }
  return true;
}

bool employ_players_db::deinit(const std::string &sName, bool bSilent) {
  sea5kg::log::info(TAG, "deinit");
  return true;
}

std::shared_ptr<gs_mmo::player_db_info> employ_players_db::create_player() {
  std::lock_guard<std::mutex> lock(m_mutex_players_db);
  auto row = std::make_shared<gs_mmo::player_db_info>();
  std::string secret = generate_random_secret();
  while (m_cache_secret_players.count(secret) > 0) {
    sea5kg::log::warning(TAG, "secret already exists (debug)");
    secret = generate_random_secret();
  }
  auto now = std::chrono::system_clock::now().time_since_epoch();
  long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

  row->secret = secret;
  row->created_dt = now_ms;
  row->updated_dt = now_ms;
  row->pos_x = 0;
  row->pos_y = 0;
  std::string sQuery = "INSERT INTO players(player_name, secret, created_dt, updated_dt, pos_x, pos_y) "
                       "VALUES('" +
                       row->player_name + "', '" + row->secret + "', " + std::to_string(row->created_dt) + ", " +
                       std::to_string(row->updated_dt) + ", 0, 0);";
  std::string error;
  if (!m_players_db->execute_query(sQuery, error)) {
    sea5kg::log::error(TAG, error);
    return nullptr;
  }
  row->id = m_players_db->select_sum_or_count("select seq from sqlite_sequence where name='players'", error);
  sea5kg::log::debug(TAG, "row->id " + std::to_string(row->id));
  if (row->id == -1) {
    sea5kg::log::error(TAG, error);
    return nullptr;
  }
  row->player_name = "p" + std::to_string(row->id);
  sQuery = "UPDATE players SET player_name = '" + row->player_name + "' WHERE id = " + std::to_string(row->id);
  if (!m_players_db->execute_query(sQuery, error)) {
    sea5kg::log::error(TAG, error);
    return nullptr;
  }

  m_cache_id_players[row->id] = row;
  m_cache_secret_players[row->secret] = row;
  return row;
}

std::shared_ptr<gs_mmo::player_db_info> employ_players_db::get_player_by_secret(const std::string &secret) {
  std::lock_guard<std::mutex> lock(m_mutex_players_db);
  if (m_cache_secret_players.count(secret) == 0) {
    return nullptr;
  }
  return m_cache_secret_players[secret];
}

void employ_players_db::update_last_position(int id, int x, int y, bool force_update) {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

  if (m_cache_id_players[id]->pos_x == x && m_cache_id_players[id]->pos_y == y) {
    // no need update
    return;
  }

  // update per sec
  if (now_ms - m_cache_id_players[id]->updated_dt < 1000 && !force_update) {
    // skipped, because if it updated no long time ago
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex_players_db);
  std::string query = "UPDATE players SET pos_x = " + std::to_string(x) + ", pos_y = " + std::to_string(y) +
                      ", updated_dt = " + std::to_string(now_ms) + "  WHERE id = " + std::to_string(id);
  std::string error;
  if (!m_players_db->execute_query(query, error)) {
    sea5kg::log::error(TAG, error);
  }
  m_cache_id_players[id]->pos_x = x;
  m_cache_id_players[id]->pos_y = y;
  m_cache_id_players[id]->updated_dt = now_ms;
}

CLASS_DATABASE_UPDATE_BEGIN(database_players, initial, v001, "Init table players") {
  // IF NOT EXISTS
  return db->execute_query(
    "CREATE TABLE IF NOT EXISTS players ( "
    "  id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "  player_name VARCHAR(50) NOT NULL, "
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

std::string employ_players_db::generate_random_secret() {
  std::string ret;
  const int size = 50;
  // static const std::string sAlphabet =
  //   "0123456789"
  //   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  //   "abcdefghijklmnopqrstuvwxyz";
  while (ret.size() < size) {
    // ret += sAlphabet[rand() % sAlphabet.length()];
    const int buffer_size = 16;
    unsigned char buffer[buffer_size];
    if (RAND_bytes(buffer, buffer_size) != 1) {
      sea5kg::log::critical("flag_id_generator_random_string", "Problem with RAND_bytes");
    }
    for (int i = 0; i < buffer_size; ++i) {
      if (ret.size() >= size) {
        break;
      }
      char c = buffer[i];
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
        ret += c;
      }
    }
  }
  return ret;
}