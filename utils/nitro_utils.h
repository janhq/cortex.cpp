#pragma once
#include "cstdio"
#include "random"
#include "string"
#include <algorithm>
#include <drogon/HttpResponse.h>
#include <iostream>
#include <ostream>

namespace nitro_utils {
inline std::string generate_random_string(std::size_t length) {
  const std::string characters =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::random_device rd;
  std::mt19937 generator(rd());

  std::uniform_int_distribution<> distribution(0, characters.size() - 1);

  std::string random_string(length, '\0');
  std::generate_n(random_string.begin(), length,
                  [&]() { return characters[distribution(generator)]; });

  return random_string;
}

inline void nitro_logo() {
  std::string rainbowColors[] = {
      "\033[93m", // Yellow
      "\033[94m", // Blue
  };

  std::string resetColor = "\033[0m";
  std::string asciiArt =
      "      ___                                   ___           ___     \n"
      "     /__/\        ___           ___        /  /\\         /  /\\    \n"
      "     \\  \\:\\      /  /\\         /  /\\      /  /::\\       /  /::\\  "
      " \n"
      "      \\  \\:\\    /  /:/        /  /:/     /  /:/\\:\\     /  /:/\\:\\ "
      " \n"
      "  _____\\__\\:\\  /__/::\\       /  /:/     /  /:/  \\:\\   /  /:/  "
      "\\:\\ \n"
      " /__/::::::::\\ \\__\\/\\:\\__   /  /::\\    /__/:/ /:/___ /__/:/ "
      "\\__\\:\\\n"
      " \\  \\:\\~~\\~~\\/    \\  \\:\\/\\ /__/:/\\:\\   \\  \\:\\/:::::/ \\  "
      "\\:\\ /  /:/\n"
      "  \\  \\:\\  ~~~      \\__\\::/ \\__\\/  \\:\\   \\  \\::/~~~~   \\  "
      "\\:\\  /:/ \n"
      "   \\  \\:\\          /__/:/       \\  \\:\\   \\  \\:\\        \\  "
      "\\:\\/:/  \n"
      "    \\  \\:\\         \\__\\/         \\__\\/    \\  \\:\\        \\  "
      "\\::/   \n"
      "     \\__\\/                                 \\__\\/         \\__\\/    "
      "\n";

  int colorIndex = 0;

  for (char c : asciiArt) {
    if (c == '\n') {
      std::cout << resetColor << c;
      colorIndex = 0;
    } else {
      std::cout << rainbowColors[colorIndex % 2] << c;
      colorIndex++;
    }
  }

  std::cout << resetColor; // Reset color at the endreturn;
}

inline drogon::HttpResponsePtr nitroHttpResponse() {
  auto resp = drogon::HttpResponse::newHttpResponse();
#ifdef ALLOW_ALL_CORS
  LOG_INFO << "Respond for all cors!";
  resp->addHeader("Access-Control-Allow-Origin", "*");
#endif
  return resp;
}

inline drogon::HttpResponsePtr nitroHttpJsonResponse(const Json::Value &data) {
  auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
#ifdef ALLOW_ALL_CORS
  LOG_INFO << "Respond for all cors!";
  resp->addHeader("Access-Control-Allow-Origin", "*");
#endif
  return resp;
};

inline drogon::HttpResponsePtr nitroStreamResponse(
    const std::function<std::size_t(char *, std::size_t)> &callback,
    const std::string &attachmentFileName = "") {
  auto resp =
      drogon::HttpResponse::newStreamResponse(callback, attachmentFileName);
#ifdef ALLOW_ALL_CORS
  LOG_INFO << "Respond for all cors!";
  resp->addHeader("Access-Control-Allow-Origin", "*");
#endif
  return resp;
}

} // namespace nitro_utils
