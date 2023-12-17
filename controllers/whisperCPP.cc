#include "whisperCPP.h"
#include "whisper.h"
// #include "llama.h"

#include "utils/nitro_utils.h"

// Add definition of your processing function here
void whisperCPP::transcription(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
  MultiPartParser partParser;
  Json::Value jsonResp;

  if (partParser.parse(req) != 0 || partParser.getFiles().size() != 1)
  {
    auto resp = HttpResponse::newHttpResponse();
    resp->setBody("Must have exactly one file");
    resp->setStatusCode(k403Forbidden);
    callback(resp);
    return;
  }
  auto &file = partParser.getFiles()[0];
  const auto &formFields = partParser.getParameters();
  std::string model = formFields.at("model");
  file.save();

  jsonResp["text"] = "handling text";

  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(resp);
  return;
}

void whisperCPP::translation(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
  MultiPartParser partParser;
  Json::Value jsonResp;

  if (partParser.parse(req) != 0 || partParser.getFiles().size() != 1)
  {
    auto resp = HttpResponse::newHttpResponse();
    resp->setBody("Must have exactly one file");
    resp->setStatusCode(k403Forbidden);
    callback(resp);
    return;
  }
  auto &file = partParser.getFiles()[0];
  const auto &formFields = partParser.getParameters();
  std::string model = formFields.at("model");
  file.save();

  jsonResp["text"] = "handling text";

  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(resp);
  return;
}