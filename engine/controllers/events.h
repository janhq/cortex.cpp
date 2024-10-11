#include <drogon/HttpAppFramework.h>
#include <drogon/HttpController.h>
#include <drogon/PubSubService.h>
#include <drogon/WebSocketController.h>
#include <eventpp/eventqueue.h>
#include <unordered_set>
#include "common/event.h"

using namespace drogon;

using Event = cortex::event::Event;
using ExitEvent = cortex::event::ExitEvent;
using DownloadEvent = cortex::event::DownloadEvent;
using EventType = cortex::event::EventType;
using EventQueue =
    eventpp::EventQueue<EventType, void(const eventpp::AnyData<eventMaxSize>&)>;

class Events : public drogon::WebSocketController<Events, false> {

 public:
  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/events", Get);
  WS_PATH_LIST_END

  explicit Events(std::shared_ptr<EventQueue> event_queue)
      : event_queue_{event_queue} {
    event_queue_->appendListener(
        EventType::DownloadEvent,
        [this](const DownloadEvent& e) { this->broadcast(e.ToJsonString()); });

    event_queue_->appendListener(
        EventType::ExitEvent,
        [this](const ExitEvent& e) { this->broadcast(e.message); });
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
  std::unordered_set<WebSocketConnectionPtr> connections_;
};
