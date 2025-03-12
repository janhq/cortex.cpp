#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "utils/string_utils.h"

namespace cli_selection_utils {
const std::string indent = std::string(4, ' ');
inline void PrintMenu(
    const std::vector<std::string>& options,
    const std::optional<std::string> default_option = std::nullopt,
    const int start_index = 1) {
  auto index{start_index};
  for (const auto& option : options) {
    bool is_default = false;
    if (default_option.has_value() &&
        string_utils::EqualsIgnoreCase(option, default_option.value())) {
      is_default = true;
    }
    std::string selection{std::to_string(index) + ". " + option +
                          (is_default ? " (default)" : "") + "\n"};
    std::cout << indent << selection;
    index++;
  }
  std::endl(std::cout);
}

inline std::optional<int> GetNumericValue(const std::string& sval) {
  try {
      return std::stoi(sval);
  } catch (const std::invalid_argument&) {
      // Not a valid number
      return std::nullopt; 
  } catch (const std::out_of_range&) {
      // Number out of range
      return std::nullopt;
  }
}

inline std::optional<std::string> PrintModelSelection(
    const std::vector<std::string>& downloaded,
    const std::vector<std::string>& availables,
    const std::optional<std::string> default_selection = std::nullopt) {

  std::string selection;
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
    if (default_selection.has_value()) {
      for (const auto& available : availables) {
        if (string_utils::EqualsIgnoreCase(available,
                                           default_selection.value())) {
          return available;
        }
      }
    }
    return std::nullopt;
  }

  // Validate if the selection consists solely of numeric characters
  if(!std::all_of(selection.begin(), selection.end(), ::isdigit)){
    return std::nullopt;
  }

  // deal with out of range numeric values
  std::optional<int> numeric_value = GetNumericValue(selection);
  
  if (!numeric_value.has_value() || numeric_value.value() > availables.size() || numeric_value.value() < 1) {
    return std::nullopt;
  }

  return availables[std::stoi(selection) - 1];
}

inline std::optional<std::string> PrintSelection(
    const std::vector<std::string>& options,
    const std::string& title = "Select an option") {
  std::cout << title << "\n";
  std::string selection;
  PrintMenu(options);
  std::cout << "Select an option (" << 1 << "-" << options.size() << "): ";
  std::getline(std::cin, selection);

  if (selection.empty()) {
    return std::nullopt;
  }

  // Validate if the selection consists solely of numeric characters
  if(!std::all_of(selection.begin(), selection.end(), ::isdigit)){
    return std::nullopt;
  }
  
  // deal with out of range numeric values
  std::optional<int> numeric_value = GetNumericValue(selection);
  if (!numeric_value.has_value() || numeric_value.value() > options.size() || numeric_value.value() < 1) {
    return std::nullopt;
  }

  return options[std::stoi(selection) - 1];
}
}  // namespace cli_selection_utils
