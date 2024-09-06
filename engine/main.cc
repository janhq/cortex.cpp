#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <climits>  // for PATH_MAX
#include <cstddef>
#include <exception>
#include <iostream>
#include <ostream>
#include "controllers/command_line_parser.h"
#include "cortex-common/cortexpythoni.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/dylib.h"
#include "utils/logging_utils.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <libgen.h>  // for dirname()
#include <mach-o/dyld.h>
#include <sys/types.h>
#elif defined(__linux__)
#include <libgen.h>  // for dirname()
#include <sys/types.h>
#include <unistd.h>  // for readlink()
#elif defined(_WIN32)
#include <afunix.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#undef max
#else
#error "Unsupported platform!"
#endif

#ifdef _WIN32

#define SOCKET_PATH "cortex_socket"
#define BUFFER_SIZE 1024
std::atomic<bool> server_running(true);

void CLISendLogMessage(const char* msg, const uint64_t len) {
  bool socket_live = true;
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed.\n";
    socket_live = false;
  }
  // Create Unix domain socket for Windows
  SOCKET sock_fd;
  struct sockaddr_un server_addr;
  sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd == INVALID_SOCKET) {
    perror("Socket creation failed");
    WSACleanup();
    socket_live = false;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_UNIX;
  strncpy_s(server_addr.sun_path, SOCKET_PATH,
            sizeof(server_addr.sun_path) - 1);

  if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Connect failed");
    closesocket(sock_fd);
    WSACleanup();
    socket_live = false;
  }
  if (socket_live) {
    size_t bytes_sent = send(sock_fd, msg, len, 0);
  }
  shutdown(sock_fd, SD_BOTH);
  closesocket(sock_fd);
  WSACleanup();
}
int SocketProcessWindows() {
  // this process will write log to file
  std::cout << "Socket creating" << std::endl;

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed.\n";
    return 1;
  }

  // Create Unix domain socket for Windows
  SOCKET server_fd;
  struct sockaddr_un server_addr {};

  if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    perror("Socket creation failed");
    WSACleanup();
    return 1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_UNIX;
  strncpy_s(server_addr.sun_path, SOCKET_PATH,
            sizeof(server_addr.sun_path) - 1);
  _unlink(SOCKET_PATH);  // Ensure the socket file does not already exist

  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind socket failed");
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    perror("Listen failed");
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  std::cout << "Server waiting for connections on Unix domain socket..."
            << std::endl;
  while (server_running) {
    SOCKET client_fd;
    if ((client_fd = accept(server_fd, nullptr, nullptr)) < 0) {
      perror("Accept failed");
      continue;
    }
    {
      char buffer[BUFFER_SIZE];

      // Receive message from client
      int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_read > 0) {
        LOG_RAW << std::string(buffer, bytes_read);
      }
      shutdown(client_fd, SD_BOTH);
      closesocket(client_fd);
    }
  }

  closesocket(server_fd);
  WSACleanup();
  return 0;
}
#endif

void RunServer() {
// Create logs/ folder and setup log to file
#ifdef _WIN32
  // if windows, we will create client socket to send to log process, create thread to run log process
  std::thread socket_thread(SocketProcessWindows);
  socket_thread.detach();
  std::cout << "Starting server ... \n";
#endif
  std::filesystem::create_directory(cortex_utils::logs_folder);
  // auto asyncFileLogger = std::make_shared<trantor::AsyncFileLogger>();
  trantor::AsyncFileLogger asyncFileLogger;
  asyncFileLogger.setFileName(cortex_utils::logs_base_name);
  asyncFileLogger.startLogging();
  trantor::Logger::setOutputFunction(
      [&](const char* msg, const uint64_t len) {
        asyncFileLogger.output(msg, len);
      },
      [&]() { asyncFileLogger.flush(); });
  asyncFileLogger.setFileSizeLimit(cortex_utils::log_file_size_limit);
  int thread_num = 1;
  std::string host = "127.0.0.1";
  int port = 3928;

  int logical_cores = std::thread::hardware_concurrency();
  int drogon_thread_num = std::max(thread_num, logical_cores);
  // cortex_utils::nitro_logo();
#ifdef CORTEX_CPP_VERSION
  LOG_INFO << "cortex.cpp version: " << CORTEX_CPP_VERSION;
#else
  LOG_INFO << "cortex.cpp version: undefined";
#endif

  LOG_INFO << "Server started, listening at: " << host << ":" << port;
  LOG_INFO << "Please load your model";
  drogon::app().addListener(host, port);
  drogon::app().setThreadNum(drogon_thread_num);
  LOG_INFO << "Number of thread is:" << drogon::app().getThreadNum();

  drogon::app().run();
#ifdef _WIN32
  server_running = false;
  std::cout << "socket closed" << std::endl;
#endif
}

void ForkProcess() {
#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  std::string cmds =
      cortex_utils::GetCurrentPath() + "/cortex-cpp.exe --start-server";
  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          const_cast<char*>(
              cmds.c_str()),  // Command line (replace with your actual executable)
          NULL,               // Process handle not inheritable
          NULL,               // Thread handle not inheritable
          FALSE,              // Set handle inheritance to FALSE
          0,                  // No creation flags
          NULL,               // Use parent's environment block
          NULL,               // Use parent's starting directory
          &si,                // Pointer to STARTUPINFO structure
          &pi))               // Pointer to PROCESS_INFORMATION structure
  {
    std::cout << "Could not start server: " << GetLastError() << std::endl;
  } else {
    std::cout << "Server started" << std::endl;
  }

#else
  // Unix-like system-specific code to fork a child process
  pid_t pid = fork();

  if (pid < 0) {
    // Fork failed
    std::cerr << "Could not start server: " << std::endl;
    return;
  } else if (pid == 0) {
    // Child process
    RunServer();
  } else {
    // Parent process
    std::cout << "Server started" << std::endl;
  }
#endif
}

int main(int argc, char* argv[]) {
  // Check if this process is for python execution
  if (argc > 1) {
    if (strcmp(argv[1], "--run_python_file") == 0) {
      std::string py_home_path = (argc > 3) ? argv[3] : "";
      std::unique_ptr<cortex_cpp::dylib> dl;
      try {
        std::string abs_path = cortex_utils::GetCurrentPath() +
                               cortex_utils::kPythonRuntimeLibPath;
        dl = std::make_unique<cortex_cpp::dylib>(abs_path, "engine");
      } catch (const cortex_cpp::dylib::load_error& e) {
        LOG_ERROR << "Could not load engine: " << e.what();
        return 1;
      }

      auto func = dl->get_function<CortexPythonEngineI*()>("get_engine");
      auto e = func();
      e->ExecutePythonFile(argv[0], argv[2], py_home_path);
      return 0;
    } else if (strcmp(argv[1], "--start-server") == 0) {
      RunServer();
      return 0;
    } else {
      // if --verbose flag log all to terminal
      bool verbose = false;
      for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
          verbose = true;
          break;
        }
      }
      if (!verbose) {  // if no flag verbose, set up to write log to file
#ifdef _WIN32
        trantor::Logger::setOutputFunction(
            [](const char* msg, const uint64_t len) {
              // Send message to server
              CLISendLogMessage(msg, len);
            },
            []() {});

#else  // linux and mac, write directly to log file
        std::filesystem::create_directory(cortex_utils::logs_folder);
        // auto asyncFileLogger = std::make_shared<trantor::AsyncFileLogger>();
        trantor::AsyncFileLogger asyncFileLogger;
        asyncFileLogger.setFileName(cortex_utils::logs_base_name);
        asyncFileLogger.startLogging();
        trantor::Logger::setOutputFunction(
            [&](const char* msg, const uint64_t len) {
              asyncFileLogger.output(msg, len);
            },
            [&]() { asyncFileLogger.flush(); });
        asyncFileLogger.setFileSizeLimit(cortex_utils::log_file_size_limit);
#endif
      }
      CommandLineParser clp;
      clp.SetupCommand(argc, argv);
      return 0;
    }
  }

  ForkProcess();
  return 0;
}
