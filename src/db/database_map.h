#ifndef DATABASE_MAP_H
#define DATABASE_MAP_H

#include <string>
#include "third_party/nlohmann/json.hpp"
#include <sqlite3.h>
#include "game_map_object.h"

class DatabaseMap {
    public:
        DatabaseMap(const std::string &sFilepath);
        bool connect();

        std::vector<GameMapObject *> getMapObjectsRect(int x, int y, int w, int h);

    private:
        std::string TAG;
        std::string m_sFilepath;
        sqlite3 *m_pDatabase;
};

#endif // DATABASE_MAP_H