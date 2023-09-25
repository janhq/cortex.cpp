#pragma once

#include "drogon/HttpTypes.h"
#include <drogon/HttpSimpleController.h>

using namespace drogon;

class helloworld : public drogon::HttpSimpleController<helloworld>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    // list path definitions here;
    PATH_ADD("/test",Get);
    PATH_LIST_END
};
