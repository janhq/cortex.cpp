#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "utils/logging_utils.h"

namespace cli_selection_utils {
const std::string indent = std::string(4, ' ');
inline void PrintMenu(
    const std::vector<std::string>& options,
    const std::optional<std::string> default_option = std::nullopt,
    const int start_index = 1) {
  auto index{start_index};
  for (const auto& option : options) {
    bool is_default = false;
    if (default_option.has_value() && option == default_option.value()) {
      is_default = true;
    }
    std::string selection{std::to_string(index) + ". " + option +
                          (is_default ? " (default)" : "") + "\n"};
    std::cout << indent << selection;
    index++;
  }
  std::endl(std::cout);
}

inline std::optional<std::string> PrintModelSelection(
    const std::vector<std::string>& downloaded,
    const std::vector<std::string>& availables,
    const std::optional<std::string> default_selection = std::nullopt) {

  std::string selection{""};
  if (!downloaded.empty()) {
    std::cout << "Downloaded models:\n";
    for (const auto& option : downloaded) {
      std::cout << indent << option << "\n";
    }
    std::endl(std::cout);
  }

  if (!availables.empty()) {
    std::cout << "Available to download:\n";
    PrintMenu(availables, default_selection, 1);
  }

  std::cout << "Select a model (" << 1 << "-" << availables.size() << "): ";
  std::getline(std::cin, selection);

  // if selection is empty and default selection is inside availables, return default_selection
  if (selection.empty()) {
    if (default_selection.has_value() &&
        std::find(availables.begin(), availables.end(),
                  default_selection.value()) != availables.end()) {
      return default_selection;
    }
    return std::nullopt;
  }

  if (std::stoi(selection) > availables.size() || std::stoi(selection) < 1) {
    return std::nullopt;
  }

  return availables[std::stoi(selection) - 1];
}

inline std::optional<std::string> PrintSelection(
    const std::vector<std::string>& options,
    const std::string& title = "Select an option") {
  std::cout << title << "\n";
  std::string selection{""};
  PrintMenu(options);
  std::cout << "Select an option (" << 1 << "-" << options.size() << "): ";
  std::getline(std::cin, selection);

  if (selection.empty()) {
    return std::nullopt;
  }

  if (std::stoi(selection) > options.size() || std::stoi(selection) < 1) {
    return std::nullopt;
  }

  return options[std::stoi(selection) - 1];
}
}  // namespace cli_selection_utils
