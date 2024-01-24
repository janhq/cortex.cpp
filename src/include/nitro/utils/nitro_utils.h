#pragma once
#include "cstdio"
#include "random"
#include "string"
#include <algorithm>
#include <drogon/HttpResponse.h>
#include <iostream>
#include <ostream>
#include <regex>
// Include platform-specific headers
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#else
#include <dirent.h>
#endif

namespace nitro_utils {

inline std::string models_folder = "./models";

inline std::string extractBase64(const std::string &input) {
  std::regex pattern("base64,(.*)");
  std::smatch match;

  if (std::regex_search(input, match, pattern)) {
    std::string base64_data = match[1];
    base64_data = base64_data.substr(0, base64_data.length() - 1);
    return base64_data;
  }

  return "";
}

inline std::vector<std::string> listFilesInDir(const std::string &path) {
  std::vector<std::string> files;

#ifdef _WIN32
  // Windows-specific code
  WIN32_FIND_DATA findFileData;
  HANDLE hFind = FindFirstFile((path + "\\*").c_str(), &findFileData);

  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        files.push_back(findFileData.cFileName);
      }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
  }
#else
  // POSIX-specific code (Linux, Unix, MacOS)
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir(path.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_REG) { // Check if it's a regular file
        files.push_back(ent->d_name);
      }
    }
    closedir(dir);
  }
#endif

  return files;
}

inline std::string rtrim(const std::string &str) {
  size_t end = str.find_last_not_of("\n\t ");
  return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

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
