#include <drogon/HttpAppFramework.h>
#include <drogon/HttpController.h>
#include <drogon/PubSubService.h>
#include <drogon/WebSocketController.h>
#include <eventpp/eventqueue.h>
#include "common/download_event.h"
#include "common/event.h"

using namespace drogon;

using Event = cortex::event::Event;
using DownloadEvent = cortex::event::DownloadEvent;
using EventQueue = eventpp::EventQueue<std::string, void(DownloadEvent)>;

class Events : public drogon::WebSocketController<Events, false> {

 public:
  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/events", Get);
  WS_PATH_LIST_END

  explicit Events(std::shared_ptr<EventQueue> event_queue)
      : event_queue_{event_queue} {
    // TODO: namh make a list of event
    event_queue_->appendListener("download-update", [this](DownloadEvent e) {
      this->broadcast(e.ToJsonString());
    });
  };

  void handleNewMessage(const WebSocketConnectionPtr& wsConnPtr,
                        std::string&& message,
                        const WebSocketMessageType& type) override;

  void handleNewConnection(const HttpRequestPtr& req,
                           const WebSocketConnectionPtr& wsConnPtr) override;

  void handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) override;

 private:
  void broadcast(const std::string& message);

  std::shared_ptr<EventQueue> event_queue_;
  std::set<WebSocketConnectionPtr> connections_;
};
