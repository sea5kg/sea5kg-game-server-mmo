#ifndef VV_WS_SERVER_H
#define VV_WS_SERVER_H

#include <json.hpp>
#include <string>

#include "EventLoop.h"
#include "WebSocketServer.h"
#include "game_map_objects.h"
#include "hssl.h"
#include "htime.h"
#include "player_context.h"

class VvWsConnectionContext {
public:
  VvWsConnectionContext();
  ~VvWsConnectionContext();
  int handleMessage(const std::string &msg);
  hv::TimerID &getTimerId();
  void setTimerId(hv::TimerID &timerID);
  PlayerContext *getPlayerContext();
  void setPlayerContext(PlayerContext *);

private:
  hv::TimerID m_nTimerID;
  PlayerContext *m_pPlayerContext;
};

class VvWsServer {
public:
  VvWsServer(GameMapObjects *pGameMapObjects);
  WebSocketService *getService();

private:
  void onMessage(const WebSocketChannelPtr &channel, const std::string &msg);

  std::string TAG;
  WebSocketService m_wsService;
  GameMapObjects *m_pGameMapObjects;
  std::map<std::string, PlayerContext *> m_mapSessions;
};

#endif // VV_WS_SERVER_H