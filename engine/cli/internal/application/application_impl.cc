// commands/internal/application/application_impl.cc
#include "public/application.h"
#include "application_impl.h"
#include "public/command_registry.h"
#include "internal/chat/chat_completion_cmd.h"

#include <iostream>
#include "services/database_service.h"
#include "services/download_service.h"
#include "utils/system_info_utils.h"

namespace cortex {

Application::Application() : impl_(std::make_unique<Impl>()) {
    // Application constructor initializes the implementation
}

Application::~Application() = default;

int Application::Run(int argc, char* argv[]) {
    // Check system compatibility
    if (!impl_->IsSystemSupported()) {
        std::cerr << "Unsupported system" << std::endl;
        return 1;
    }
    
    // Initialize system
    if (!impl_->InitializeSystem()) {
        std::cerr << "Failed to initialize system" << std::endl;
        return 1;
    }
    
    // Initialize services
    if (!impl_->InitializeServices()) {
        std::cerr << "Failed to initialize services" << std::endl;
        return 1;
    }
    
    // Register commands
    impl_->RegisterCommands();
    
    // Parse command line arguments
    std::string command_name;
    std::vector<std::string> args;
    std::unordered_map<std::string, std::string> options;
    impl_->ParseArgs(argc, argv, command_name, args, options);
    
    // Handle help command or no command
    if (command_name.empty() || command_name == "help") {
        if (args.empty()) {
            impl_->command_registry_->PrintHelp();
        } else {
            impl_->command_registry_->PrintCommandHelp(args[0]);
        }
        return 0;
    }
    
    // Execute the command
    auto status = impl_->command_registry_->ExecuteCommand(command_name, args, options);
    
    // Cleanup
    impl_->CleanupSystem();
    
    return status == commands::CommandStatus::Success ? 0 : 1;
}

int Application::RunCommand(const std::string& command, const std::vector<std::string>& args) {
    // Execute a specific command programmatically
    if (!impl_->command_registry_) {
        if (!impl_->InitializeServices()) {
            return 1;
        }
        impl_->RegisterCommands();
    }
    
    auto status = impl_->command_registry_->ExecuteCommand(command, args, {});
    return status == commands::CommandStatus::Success ? 0 : 1;
}

// Implementation class methods

Application::Impl::Impl() = default;

Application::Impl::~Impl() = default;

bool Application::Impl::InitializeServices() {
    try {
        // Initialize services
        db_service_ = std::make_shared<DatabaseService>();
        download_service_ = std::make_shared<DownloadService>();
        
        // Create command registry
        command_registry_ = commands::CommandRegistry::Create();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing services: " << e.what() << std::endl;
        return false;
    }
}

void Application::Impl::RegisterCommands() {
    // Register chat command
    command_registry_->RegisterCommand(
        std::make_shared<commands::ChatCompletionCommand>(db_service_));
    
    // Register other commands
    // command_registry_->RegisterCommand(
    //     std::make_shared<commands::RunCommand>(db_service_));
    // ...
}

void Application::Impl::ParseArgs(int argc, char* argv[], 
                                std::string& command_name,
                                std::vector<std::string>& args,
                                std::unordered_map<std::string, std::string>& options) {
    // Extract command name (first argument)
    if (argc > 1) {
        command_name = argv[1];
    }
    
    // Process remaining arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        // Handle options (--option=value or --flag)
        if (arg.size() > 2 && arg.substr(0, 2) == "--") {
            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                // --option=value format
                std::string option_name = arg.substr(2, pos - 2);
                std::string option_value = arg.substr(pos + 1);
                options[option_name] = option_value;
            } else {
                // --flag format
                options[arg.substr(2)] = "true";
            }
        } else if (arg.size() > 1 && arg[0] == '-') {
            // Handle short options (-f)
            std::string option_name = arg.substr(1);
            
            // If next argument isn't an option, treat it as this option's value
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options[option_name] = argv[i + 1];
                ++i;  // Skip the value in the next iteration
            } else {
                // Flag option
                options[option_name] = "true";
            }
        } else {
            // Regular argument
            args.push_back(arg);
        }
    }
}

bool Application::Impl::IsSystemSupported() const {
    // Check system compatibility
    auto system_info = system_info_utils::GetSystemInfo();
    return !(system_info->arch == system_info_utils::kUnsupported ||
            system_info->os == system_info_utils::kUnsupported);
}

bool Application::Impl::InitializeSystem() {
    try {
        // Initialize libraries and resources
        // This would include things like SSL_library_init() and curl_global_init()
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing system: " << e.what() << std::endl;
        return false;
    }
}

void Application::Impl::CleanupSystem() {
    // Clean up global resources
    // This would include things like curl_global_cleanup()
}

} // namespace cortex