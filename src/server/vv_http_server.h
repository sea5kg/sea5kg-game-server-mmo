#ifndef VV_HTTP_SERVER_H
#define VV_HTTP_SERVER_H

#include <string>
#include <json.hpp>
#include "game_map_objects.h"

// libhv includes
#include "HttpService.h" // libhv
#include "WebSocketServer.h"  // libhv
#include "EventLoop.h"  // libhv
#include "htime.h"  // libhv
#include "hssl.h"  // libhv
#include "hlog.h"  // libhv

class VvHttpServer {
    public:
        VvHttpServer(GameMapObjects *pGameMapObjects);
        HttpService *getService();
        int httpApiV1GetPaths(HttpRequest* req, HttpResponse* resp);

    private:
        std::string TAG;
        HttpService *m_httpService;
        GameMapObjects *m_pGameMapObjects;
};

#endif // VV_HTTP_SERVER_H