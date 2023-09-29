#pragma once
#include "cstdio"
#include "random"
#include "string"
#include <iostream>
#include <ostream>
#include <algorithm>

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

inline void nitro_logo(){
    std::string rainbowColors[] = {
        "\033[93m",  // Yellow
        "\033[94m",  // Blue
    };

    std::string resetColor = "\033[0m";
    std::string asciiArt =
        "      ___                                   ___           ___     \n"
        "     /__/\        ___           ___        /  /\\         /  /\\    \n"
        "     \\  \\:\\      /  /\\         /  /\\      /  /::\\       /  /::\\   \n"
        "      \\  \\:\\    /  /:/        /  /:/     /  /:/\\:\\     /  /:/\\:\\  \n"
        "  _____\\__\\:\\  /__/::\\       /  /:/     /  /:/  \\:\\   /  /:/  \\:\\ \n"
        " /__/::::::::\\ \\__\\/\\:\\__   /  /::\\    /__/:/ /:/___ /__/:/ \\__\\:\\\n"
        " \\  \\:\\~~\\~~\\/    \\  \\:\\/\\ /__/:/\\:\\   \\  \\:\\/:::::/ \\  \\:\\ /  /:/\n"
        "  \\  \\:\\  ~~~      \\__\\::/ \\__\\/  \\:\\   \\  \\::/~~~~   \\  \\:\\  /:/ \n"
        "   \\  \\:\\          /__/:/       \\  \\:\\   \\  \\:\\        \\  \\:\\/:/  \n"
        "    \\  \\:\\         \\__\\/         \\__\\/    \\  \\:\\        \\  \\::/   \n"
        "     \\__\\/                                 \\__\\/         \\__\\/    \n";

    int colorIndex = 0;

    for (char c : asciiArt) {
        if (c == '\n') {
            std::cout << resetColor << c;
            colorIndex = 0;
        } else {
            std::cout << rainbowColors[colorIndex % 6] << c;
            colorIndex++;
        }
    }

    std::cout << resetColor; // Reset color at the endreturn;
}

} // namespace nitro_utils
