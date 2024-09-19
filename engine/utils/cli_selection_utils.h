#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace cli_selection_utils {
inline void PrintMenu(const std::vector<std::string>& options) {
  auto index{1};
  for (const auto& option : options) {
    std::cout << index << ". " << option << "\n";
    index++;
  }
  std::endl(std::cout);
}

inline std::optional<std::string> PrintSelection(
    const std::vector<std::string>& options,
    const std::string& title = "Select an option") {
  std::cout << title << "\n";
  std::string selection{""};
  PrintMenu(options);
  std::cout << "Select an option (" << 1 << "-" << options.size() << "): ";
  std::cin >> selection;

  if (selection.empty()) {
    return std::nullopt;
  }

  if (std::stoi(selection) > options.size() || std::stoi(selection) < 1) {
    return std::nullopt;
  }

  return options[std::stoi(selection) - 1];
}
}  // namespace cli_selection_utils
