#include <string.h>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include "game_map_objects.h"
#include <sqlite3.h>
#include "vv_server.h"
#include "database_map.h"

int main(int argc, const char* argv[]) {
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

