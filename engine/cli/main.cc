// main.cc
#include "public/application.h"
#include <openssl/ssl.h>
#include <curl/curl.h>

int main(int argc, char* argv[]) {
    // Initialize global libraries
    SSL_library_init();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Create and run the application
    int result = 0;
    {
        cortex::Application app;
        result = app.Run(argc, argv);
    }
    
    // Cleanup global resources
    curl_global_cleanup();
    
    return result;
}