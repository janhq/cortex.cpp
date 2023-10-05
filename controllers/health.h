#pragma once

#include <drogon/HttpSimpleController.h>
#include <drogon/HttpTypes.h>

using namespace drogon;

class health : public drogon::HttpSimpleController<health>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    // list path definitions here;
    PATH_ADD("/healthz", Get);
    PATH_LIST_END
};
