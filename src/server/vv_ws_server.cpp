#include "vv_ws_server.h"
#include <fstream>
#include <iostream>
#include <json.hpp>

using namespace hv;

// ---------------------------------------------------------------------
// VvWsConnectionContext

VvWsConnectionContext::VvWsConnectionContext() {
    m_nTimerID = INVALID_TIMER_ID;
    m_pPlayerContext = nullptr;
}

VvWsConnectionContext::~VvWsConnectionContext() {
}

int VvWsConnectionContext::handleMessage(const std::string& msg) {
    printf("onmessage: %s\n", msg.c_str());
    nlohmann::json j = nlohmann::json::parse(msg);
    if (j["method"] == "player_move_to") {
        if (m_pPlayerContext) {
            int m_nX = j["params"]["x"];
            int m_nY = j["params"]["y"];
            m_pPlayerContext->setPlayerX(m_nX);
            m_pPlayerContext->setPlayerY(m_nY);
            printf("new coordinates: %d,%d\n", m_nX, m_nY);
        } else {
            std::cerr << "Missing player context" << std::endl;
        }
    }
    return 0;
}

hv::TimerID &VvWsConnectionContext::getTimerId() {
    return m_nTimerID;
}

void VvWsConnectionContext::setTimerId(hv::TimerID &timerID) {
    m_nTimerID = timerID;
}

PlayerContext *VvWsConnectionContext::getPlayerContext() {
    return m_pPlayerContext;
}

void VvWsConnectionContext::setPlayerContext(PlayerContext *pPlayerContext) {
    m_pPlayerContext = pPlayerContext;
}

// ---------------------------------------------------------------------
// VVWsServer

VvWsServer::VvWsServer(GameMapObjects *pGameMapObjects) {
    TAG = "VvWsServer";
    m_pGameMapObjects = pGameMapObjects;

    // // m_wsService = new WebSocketService();
    // m_wsService.onopen = [](const WebSocketChannelPtr& channel, const std::string& url) {
    //     printf("onopen: GET %s\n", url.c_str());
    //     VvWsConnectionContext* ctx = channel->newContext<VvWsConnectionContext>();
    //     // send(time) every 1s
    //     hv::TimerID timerID = hv::setInterval(1000, [channel](hv::TimerID id) {
    //         char str[DATETIME_FMT_BUFLEN] = {0};
    //         datetime_t dt = datetime_now();
    //         datetime_fmt(&dt, str);
    //         nlohmann::json jsonResp;
    //         jsonResp["id"] = "sync";
    //         jsonResp["result"] = str;
    //         channel->send(jsonResp.dump());
    //     });
    //     ctx->setTimerId(timerID);
    // };

    // m_wsService.onmessage = std::bind(&VvWsServer::onMessage, this, std::placeholders::_1, std::placeholders::_2);

    // m_wsService.onclose = [](const WebSocketChannelPtr& channel) {
    //     // printf("onclose\n");
    //     VvWsConnectionContext* ctx = channel->getContext<VvWsConnectionContext>();
    //     if (ctx->getTimerId() != INVALID_TIMER_ID) {
    //         killTimer(ctx->getTimerId());
    //     }
    //     channel->deleteContext<VvWsConnectionContext>();
    // };

}

WebSocketService *VvWsServer::getService() {
    return &m_wsService;
}

void VvWsServer::onMessage(const WebSocketChannelPtr& channel, const std::string& msg) {
    VvWsConnectionContext* ctx = channel->getContext<VvWsConnectionContext>();
    std::cout << "msg = " << msg << std::endl;
    // https://www.jsonrpc.org/specification
    nlohmann::json j = nlohmann::json::parse(msg);
    std::string sMethod = j["method"];
    if (sMethod == "get_player_position") {
        nlohmann::json jsonResp;
        jsonResp["id"] = j["id"];
        jsonResp["method"] = sMethod;
        
        if (ctx->getPlayerContext()) {
            nlohmann::json jsonResult;
            jsonResult["x"] = ctx->getPlayerContext()->getPlayerX();
            jsonResult["y"] = ctx->getPlayerContext()->getPlayerY();
            jsonResp["result"] = jsonResult;
        } else {
            nlohmann::json jsonError;
            jsonError["code"] = 550;
            jsonError["message"] = "Not found error context";
            jsonResp["error"] = jsonError;
        }
        channel->send(jsonResp.dump());
    } else if (j["method"] == "player_move_to") {
        ctx->handleMessage(msg);
    } else if (sMethod == "set_session_token") {
        nlohmann::json jsonResp;
        jsonResp["id"] = j["id"];
        std::string sToken = j["params"]["token"];
        if (m_mapSessions.count(sToken) <= 0) {
            m_mapSessions[sToken] = new PlayerContext();
        }
        ctx->setPlayerContext(m_mapSessions[sToken]);
        nlohmann::json jsonResult;
        jsonResp["result"] = jsonResult;
        channel->send(jsonResp.dump());
    } else if (sMethod == "get_map") {
        nlohmann::json jsonResp;
        jsonResp["id"] = j["id"];
        jsonResp["method"] = sMethod;
        if (ctx->getPlayerContext()) {
            nlohmann::json jsonResult;
            int nX = ctx->getPlayerContext()->getPlayerX();
            int nY = ctx->getPlayerContext()->getPlayerY();
            jsonResp["result"] = m_pGameMapObjects->toJson(nX - 1000, nY - 1000, 2000, 2000);
        } else {
            nlohmann::json jsonError;
            jsonError["code"] = 550;
            jsonError["message"] = "Not found error context";
            jsonResp["error"] = jsonError;
        }
        channel->send(jsonResp.dump());
    } else {
        std::cout << "Unknown " << sMethod << std::endl;
    }
}