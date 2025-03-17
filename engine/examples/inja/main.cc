#include "extensions/template_renderer.h"
#include "utils/json_helper.h"

void print_help() {
  std::cout << "Usage: \ninja-test [options]\n\n";
  std::cout << "Options:\n";
  std::cout << "  --template  Path to template file\n";
  std::cout << "  --data      Path to data file\n";

  exit(0);
}

int main(int argc, char* argv[]) {
  std::filesystem::path template_path;
  std::filesystem::path data_path;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--template") == 0) {
      template_path = argv[i + 1];
    } else if (strcmp(argv[i], "--data") == 0) {
      data_path = argv[i + 1];
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      print_help();
    }
  }

  auto get_str = [](const std::filesystem::path& p) {
    std::ifstream ifs(p);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
  };

  std::string tpl = get_str(template_path);
  std::string message = get_str(data_path);
  auto data = json_helper::ParseJsonString(message);

  extensions::TemplateRenderer rdr;
  auto res = rdr.Render(tpl, data);
  std::cout << std::endl;
  std::cout << "Result: " << std::endl;
  std::cout << res << std::endl;
  std::cout << std::endl;

  return 0;
}