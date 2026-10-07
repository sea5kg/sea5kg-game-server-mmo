#include <string.h>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include "game_map_objects.h"
#include <sqlite3.h>
#include "vv_server.h"
#include "database_map.h"

int main(int argc, const char* argv[]) {

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
    WsjcppLog::setPrefixLogFile("vv-server");
    WsjcppLog::setLogDirectory(".logs");
    WsjcppCore::initRandom();

    std::string sDatabaseMapFilePath = "./data/game-map.db";
    DatabaseMap *pDatabaseMap = new DatabaseMap(sDatabaseMapFilePath);
    if (!pDatabaseMap->connect()) {
        std::cerr << "ERROR: Could not connect to SQLite version:" << sqlite3_libversion() << std::endl;
        return -1;
    }

    GameMapObjects *pGameMaps = new GameMapObjects(pDatabaseMap);

    int nPort = 1234;
    std::cout << "SQLite version:" << sqlite3_libversion() << std::endl;
    std::cout << "Starting on port: http://localhost:" << nPort << "/" << std::endl;

    VvServer server(pGameMaps);
    server.startSync(nPort);

    return 0;
}

