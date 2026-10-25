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

#include <string.h>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <wsjcpp_core.h>
#include <sea5kg_logger.h>
#include <wsjcpp_employees.h>
#include "game_map_objects.h"
#include <sqlite3.h>
#include <sea5kg_sqlite3_wrapper.h>
#include "database_map.h"
#include "gs_mmo/include/gs_mmo_globals.h"
#include "gs_mmo/include/gs_mmo_config.h"
#include "gs_mmo/include/gs_mmo_web_server.h"

std::vector<std::string> argumentsToVector(int argc, const char* argv[]) {
  std::vector<std::string> ret;
  for (int i = 0; i < argc; i++) {
    ret.push_back(std::string(argv[i]));
  }
  return ret;
}

std::string tryResolveRelativePath(const std::string &path) {
  std::string ret = path;
  if (ret.size() > 0 && ret[0] != '/') {
      ret = WsjcppCore::getCurrentDirectory() + "/" + ret;
  }
  return wsjcpp::normalize_filepath(ret);
}

bool findWorkDir(std::vector<std::string> &arguments, std::string &workDir) {
  bool found = false;

  // try find in a program arguments
  for (int i = 0; i < arguments.size(); i++) {
    std::string arg = arguments[i];
    if (arg == "--work-dir" || arg == "-work-dir" || arg == "-w" || arg == "-wd") {
      if (i + 1 < arguments.size()) {
        found = true;
        workDir = tryResolveRelativePath(arguments[i + 1]);
        arguments.erase(arguments.begin() + i, arguments.begin() + i + 2);
        break;
      }
    }
  }

  // Anyway, try read from environment. And override from args.
  if (WsjcppCore::getEnv("GS_MMO_WORKDIR", workDir)) {
    std::cout << "Working directory get from environment variable CTF01D_WORKDIR: " << workDir << std::endl;
    return true;
  }

  // try find by default paths
  if (!found) {
    std::vector<std::string> vPossibleFolders = {
      tryResolveRelativePath("./"),
      tryResolveRelativePath("./data/"),
      tryResolveRelativePath("/root/data/")
    };

    for (int i = 0; i < vPossibleFolders.size(); i++) {
      std::string sWorkDir = vPossibleFolders[i];
      if (wsjcpp::file_exists(sWorkDir + "/config.yml")) {
        found = true;
        workDir = sWorkDir;
        break;
      }
    }
    if (found) {
      std::cout << "Automatically detected working directory: " << workDir << std::endl;
    }
  }
  return found;
}

bool tryFindSingleArgument(const std::vector<std::string> &argNames, std::vector<std::string> &arguments) {
  for (int i = 0; i < arguments.size(); i++) {
    std::string arg = arguments[i];
    if (std::find(argNames.begin(), argNames.end(), arg) != argNames.end()) {
      arguments.erase(arguments.begin() + i);
      return true;
    }
  }
  return false;
}

void printHelp(const std::string &programName) {
  std::cout
    << std::endl
    << "Usage: " << programName << " [OPTIONS] COMMAND" << std::endl
    << std::endl
    << "Jury System for ctf-attack-defense" << std::endl
    << std::endl
    << "  example: 'ctf01d -w ./data_test start'" << std::endl
    << std::endl
    << "OPTIONS:" << std::endl
    << "  --work-dir, -work-dir, -wd, -w path         Custom workspace folder with configs, logging, " << std::endl
    << "                                              checker scripts and etc. (env: GS_MMO_WORKDIR)" << std::endl
    << "  --version, -version, -v, version            Print version and exit" << std::endl
    << "  --help, -help, help, -h                     Print help and exit" << std::endl
    << std::endl
    << "COMMANDS:" << std::endl
    << "  start                                  Start game." << std::endl
    << std::endl
  ;
}

bool is_root() {
  // Root always has an Effective User ID (EUID) of 0
  return geteuid() == 0;
}

bool change_privileges(int user_id) {
  std::cout << " ...Trying change privileges (setgid)" << std::endl;
  if (setgid(user_id) != 0) {
    std::cerr << " -> FAIL. Failed to set GID" << std::endl;
    return false;
  }
  std::cout << " ...Trying change privileges (setuid)" << std::endl;
  if (setuid(user_id) != 0) {
    std::cerr << " -> FAIL. Failed to set UID" << std::endl;
    return false;
  }
  std::cout << " ...Trying change privileges (verify)" << std::endl;
  if (setuid(0) == 0) {
    std::cerr << " -> FAIL. Security Risk: Privileges were not permanently dropped!" << std::endl;
    return false;
  }
  std::cout << " ...Trying change privileges (test)" << std::endl;
  if (getuid() == user_id) {
    std::cout << "-> OK. Successful changed privileges." << std::endl;
  } else {
    std::cerr << " -> FAIL. NOT CHANGED." << std::endl;
    return false;
  }
  return true;
}

bool try_apply_ctf01d_user(const std::string &work_dir) {
  // std::cout << "work_dir = " << work_dir << std::endl;
  std::string str_user;
  int user_id = 0;
  if (WsjcppCore::getEnv(gs_mmo::keys::GS_MMO_USER, str_user)) {
    std::cout << gs_mmo::keys::GS_MMO_USER << "='" << str_user << "'" << std::endl;
    try {
      user_id = std::stoi(str_user);
    } catch (const std::invalid_argument& e) {
      std::cerr << "Error: No conversion could be performed. " << gs_mmo::keys::GS_MMO_USER << "='" << str_user << "'" << std::endl;
      return false;
    } catch (const std::out_of_range& e) {
      std::cerr << "The converted value is too big for an int.. " << gs_mmo::keys::GS_MMO_USER << "='" << str_user << "'" << std::endl;
      return false;
    } catch (...) {
      std::cerr << "The converted value is too big for an int.. " << gs_mmo::keys::GS_MMO_USER << "='" << str_user << "'" << std::endl;
      return false;
    }
    if (is_root()) {
      std::cout << " ...Try change owner for '" << work_dir << "' to '" << str_user << ":" << str_user << "'" << std::endl;
      std::string cmd = "chown -R " + std::to_string(user_id) + ":" + std::to_string(user_id) + " \"" + work_dir + "\"";
      if (system(cmd.c_str()) == 0) {
        std::cout << " -> OK. Successful changed owner for data." << std::endl;
      } else {
        std::cerr << " -> FAIL. Could not change owner for directory." << std::endl;
        return false;
      }
      return change_privileges(user_id);
    } else if (geteuid() == user_id) {
      std::cout << " * OK. " << gs_mmo::keys::GS_MMO_USER << " is equal with current user" << std::endl;
    } else {
      return change_privileges(user_id);
    }
    return true;
  }
  return true;
}

int main(int argc, const char* argv[]) {
    WsjcppLog::setEnableLogFile(false);

    std::cout << " ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << " ::                                   oooooo 8                                 ::" << std::endl;
    std::cout << " ::                                   8      8                                 ::" << std::endl;
    std::cout << " ::              .oPYo. .oPYo. .oPYo. 8pPYo. 8  .o  .oPYo.                     ::" << std::endl;
    std::cout << " ::              Yb..   8oooo8 .oooo8     `8 8oP'   8    8                     ::" << std::endl;
    std::cout << " ::                'Yb. 8.     8    8     .P 8 `b.  8    8                     ::" << std::endl;
    std::cout << " ::              `YooP' `Yooo' `YooP8 `YooP' 8  `o. `YooP8                     ::" << std::endl;
    std::cout << " :::::::::::::::::.....::.....::.....::.....:..::...:....8 ::::::::::::::::::::::" << std::endl;
    std::cout << " :::::::::::::::::::::::::::::::::::::::::::::::::::::ooP'.::::::::::::::::::::::" << std::endl;
    std::cout << " :: .oPYo. .oPYo. ooYoYo. .oPYo.       .oPYo. .oPYo. oPYo. o    o .oPYo. oPYo. ::" << std::endl;
    std::cout << " :: 8    8 .oooo8 8' 8  8 8oooo8 ooooo Yb..   8oooo8 8  `' Y.  .P 8oooo8 8  `' ::" << std::endl;
    std::cout << " :: 8    8 8    8 8  8  8 8.             'Yb. 8.     8     `b..d' 8.     8     ::" << std::endl;
    std::cout << " :: `YooP8 `YooP8 8  8  8 `Yooo'       `YooP' `Yooo' 8      `YP'  `Yooo' 8     ::" << std::endl;
    std::cout << " ::::....8 :.....:..:..:..:.....::::::::.....::.....:..::::::...:::.....:..::::::" << std::endl;
    std::cout << " :::::ooP'.:::::::ooYoYo. ooYoYo. .oPYo.                                       ::" << std::endl;
    std::cout << " ::               8' 8  8 8' 8  8 8    8                                       ::" << std::endl;
    std::cout << " ::               8  8  8 8  8  8 8    8                                       ::" << std::endl;
    std::cout << " ::               8  8  8 8  8  8 `YooP'                                       ::" << std::endl;
    std::cout << " :::::::::::::::::..:..:....:..:..:.....:::::::::::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << " ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl;

    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_APP_NAME);
    std::string appVersion = std::string(WSJCPP_APP_VERSION);
    if (!wsjcpp::dir_exists(".logs")) {
        WsjcppCore::makeDir(".logs");
    }

    int driver_init_ret;
    if (!sea5kg::sqlite3_wrapper::global::init_driver_sqlite3(driver_init_ret)) {
      std::cerr << "Failed to initialize build-in sqlite3 library: " + std::to_string(driver_init_ret);
      return -1;
    }
    std::cout << "SQLite version:" << sqlite3_libversion() << std::endl;

    sea5kg::log::set_log_filename_prefix("gs_mmo_");

    // parse arguments
    std::vector<std::string> arguments = argumentsToVector(argc, argv);
    std::string programName = arguments[0];
    arguments.erase(arguments.begin());

    if (tryFindSingleArgument({"--version", "-version", "version", "-v"}, arguments)) {
        std::cout << appName << " " << appVersion << std::endl;
        return 0;
    }

    if (tryFindSingleArgument({"--help", "-help", "help", "-h"}, arguments)) {
        printHelp(programName);
        return -1;
    }

    std::string sWorkDir;
    if (!findWorkDir(arguments, sWorkDir)) {
        std::cout << "Working directory not found: " << sWorkDir << std::endl;
        return -1;
    }
    try_apply_ctf01d_user(sWorkDir);

    std::cout << "WorkDir: " << sWorkDir << std::endl;
    auto config = findWsjcppEmploy<gs_mmo::config>();
    config->set_work_dir(sWorkDir);
    config->set_app_version(appVersion);

    if (arguments.size() == 0) {
        std::cout << "Not found command. Please run '" << programName << " help'" << std::endl;
        return -1;
    }

    if (arguments.size() != 1) {
        std::cout << "Unknown arguments. Please run '" << programName << " help'" << std::endl;
        return -1;
    }

    std::string command = arguments[0];

    if (command == "start") {
        sea5kg::log::info(TAG, "Starting...");
        if (!WsjcppEmployees::init({})) {
          sea5kg::log::error(TAG, "Start failed on step init configs.");
          return -1;
        }

        // std::vector<ctf01d::service_checker_thread *> vThreads;
        // sea5kg::log::info(TAG, "Starting threads...");
        // for (unsigned int iservice = 0; iservice < config->services().size(); iservice++) {
        // ctf01d::service_config service_config = config->services()[iservice];

        // std::shared_ptr<sea5kg::logger> service_logger(sea5kg::logger::create());
        // service_logger->set_log_dirpath(sea5kg::log::log_dirpath());
        // service_logger->set_rotation_period_in_seconds(sea5kg::log::rotation_period_in_seconds());
        // service_logger->set_log_filename_prefix("checker_" + service_config.id() + "_");
        // service_logger->set_log_level_file_output(sea5kg::log_level::DEBUG);
        // service_logger->set_log_level_console_output(sea5kg::log_level::DISABLE); // only errors will be to main log
        // service_logger->set_log_level_redirect_to_global(sea5kg::log_level::ERROR);
        // service_logger->info(TAG, "Starting threads");

        // for (unsigned int i_team = 0; i_team < config->teams().size(); i_team++) {
        //     ctf01d::team_config team_config = config->teams()[i_team];

        //     // reset status to down
        //     // TODO reset must be in scoreboard and only if game in started not ended
        //     findWsjcppEmploy<ctf01d::scoreboard>()->set_service_status(team_config.id(), service_config.id(), ctf01d::service_status_cell::SERVICE_DOWN);

        //     ctf01d::service_checker_thread *thr = new ctf01d::service_checker_thread(service_logger, service_config, team_config);
        //     thr->start();
        //     vThreads.push_back(thr);
        // }
        // }
        // sea5kg::log::info(TAG, std::to_string(vThreads.size()) + " threads started");
        return findWsjcppEmploy<gs_mmo::web_server>()->start();
    }

    std::cout << "Unknown command '" << command << "'. Please run '" << programName << " help'" << std::endl;
    return -1;

    // WsjcppLog::setPrefixLogFile("game-server-mmo");
    // WsjcppLog::setLogDirectory(".logs");
    // WsjcppCore::initRandom();

    // std::string sDatabaseMapFilePath = "./data/game-map.db";
    // DatabaseMap *pDatabaseMap = new DatabaseMap(sDatabaseMapFilePath);
    // if (!pDatabaseMap->connect()) {
    //     std::cerr << "ERROR: Could not connect to SQLite version:" << sqlite3_libversion() << std::endl;
    //     return -1;
    // }

    // GameMapObjects *pGameMaps = new GameMapObjects(pDatabaseMap);

    // int nPort = 1234;
    // std::cout << "SQLite version:" << sqlite3_libversion() << std::endl;
    // std::cout << "Starting on port: http://localhost:" << nPort << "/" << std::endl;

    // VvServer server(pGameMaps);
    // server.startSync(nPort);

    return 0;
}

