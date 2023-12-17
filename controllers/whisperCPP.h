#pragma once

#include <drogon/HttpController.h>
#include "whisper.h"

using namespace drogon;

class whisperCPP : public drogon::HttpController<whisperCPP>
{
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  // METHOD_ADD(whisperCPP::get, "/{2}/{1}", Get); // path is /whisperCPP/{arg2}/{arg1}
  // METHOD_ADD(whisperCPP::your_method_name, "/{1}/{2}/list", Get); // path is /whisperCPP/{arg1}/{arg2}/list
  // ADD_METHOD_TO(whisperCPP::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
  ADD_METHOD_TO(whisperCPP::transcription, "/v1/audio/transcriptions", Post);
  ADD_METHOD_TO(whisperCPP::translation, "/v1/audio/translations", Post);
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
  // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
  void transcription(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback);

  void translation(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback);
};
