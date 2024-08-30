#include <sstream>
#include <vector>

namespace semantic_version_utils {
inline std::vector<int> SplitVersion(const std::string& version) {
  std::vector<int> parts;
  std::stringstream ss(version);
  std::string part;

  while (std::getline(ss, part, '.')) {
    parts.push_back(std::stoi(part));
  }

  while (parts.size() < 3) {
    parts.push_back(0);
  }

  return parts;
}

inline int CompareSemanticVersion(const std::string& version1,
                                  const std::string& version2) {
  std::vector<int> v1 = SplitVersion(version1);
  std::vector<int> v2 = SplitVersion(version2);

  for (size_t i = 0; i < 3; ++i) {
    if (v1[i] < v2[i])
      return -1;
    if (v1[i] > v2[i])
      return 1;
  }
  return 0;
}

// convert 11.7 to 11-7 for compatible to download url
inline std::string ConvertToPath(const std::string& version) {
  std::string result = version;
  int pos = result.find('.');
  if (pos != std::string::npos) {
    result[pos] = '-';
  }
  return result;
}
}  // namespace semantic_version_utils
