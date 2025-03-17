#include "events.h"

void Events::broadcast(const std::string& message) {
  for (const auto& conn : connections_) {
    conn->send(message);
  }
}

void Events::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr,
                              std::string&& message,
                              const WebSocketMessageType& type) {
  // ignore message sent from client
  (void) wsConnPtr;
  (void) message;
  (void) type;
}

void Events::handleNewConnection(const HttpRequestPtr& req,
                                 const WebSocketConnectionPtr& ws_conn_ptr) {
  connections_.insert(ws_conn_ptr);
  (void) req;
}

void Events::handleConnectionClosed(const WebSocketConnectionPtr& ws_conn_ptr) {
  connections_.erase(ws_conn_ptr);
}
