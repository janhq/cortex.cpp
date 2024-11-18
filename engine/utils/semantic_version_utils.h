#include <trantor/utils/Logger.h>
#include <sstream>

namespace semantic_version_utils {
struct SemVer {
  int major;
  int minor;
  int patch;
};

inline SemVer SplitVersion(const std::string& version) {
  if (version.empty()) {
    LOG_WARN << "Passed in version is empty!";
  }
  SemVer semVer = {0, 0, 0};  // default value
  std::stringstream ss(version);
  std::string part;

  int index = 0;
  while (std::getline(ss, part, '.') && index < 3) {
    int value = std::stoi(part);
    switch (index) {
      case 0:
        semVer.major = value;
        break;
      case 1:
        semVer.minor = value;
        break;
      case 2:
        semVer.patch = value;
        break;
    }
    ++index;
  }

  return semVer;
}

inline int CompareSemanticVersion(const std::string& version1,
                                  const std::string& version2) {
  SemVer v1 = SplitVersion(version1);
  SemVer v2 = SplitVersion(version2);

  if (v1.major < v2.major)
    return -1;
  if (v1.major > v2.major)
    return 1;

  if (v1.minor < v2.minor)
    return -1;
  if (v1.minor > v2.minor)
    return 1;

  if (v1.patch < v2.patch)
    return -1;
  if (v1.patch > v2.patch)
    return 1;

  return 0;
}
}  // namespace semantic_version_utils
