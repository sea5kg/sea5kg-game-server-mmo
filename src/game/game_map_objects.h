#ifndef GAME_MAP_OBJECTS_H
#define GAME_MAP_OBJECTS_H

#include "game_map_object.h"
#include "database_map.h"
#include "game_map_generator.h"

#include <string>
#include "third_party/nlohmann/json.hpp"

class GameMapCache {
    public:
        GameMapCache();
        int getCellCacheSize();
        bool hasObjects(int cx, int cy);
        void addObjects(int cx, int cy, const std::vector<GameMapObject *> &vObjects);
        const std::vector<GameMapObject *> &getObjects(int cx, int cy);

    private:
        int m_nCellCacheSize;
        std::map<int, std::map<int, std::vector<GameMapObject *>>> m_mapCache;
};

class GameMapObjects {
    public:
        GameMapObjects(DatabaseMap *pDatabaseMap);
        nlohmann::json toJson(int x, int y, int w, int h);

    private:
        std::string TAG;
        int alignmentByCell(int x, int d);
        // nlohmann::json m_jsonMap;
        DatabaseMap *m_pDatabaseMap;
        GameMapGenerator *m_pGameMapGenerator;
        
        // std::vector<GameMapObject *> m_vObjects;
        GameMapCache *m_pMapCache;
};

#endif // GAME_MAP_OBJECTS_H