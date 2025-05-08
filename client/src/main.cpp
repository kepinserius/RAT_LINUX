#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <cstring>

#include "client.h"

std::atomic<bool> running(true);

void signalHandler(int signum)
{
    std::cout << "Received signal " << signum << std::endl;
    running = false;
}

int main(int argc, char *argv[])
{
    // Register signal handler
    signal(SIGINT, signalHandler);

    // Default server address and port
    std::string server_address = "127.0.0.1";
    uint16_t server_port = DEFAULT_PORT;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-s" || arg == "--server")
        {
            if (i + 1 < argc)
            {
                server_address = argv[++i];
            }
        }
        else if (arg == "-p" || arg == "--port")
        {
            if (i + 1 < argc)
            {
                server_port = static_cast<uint16_t>(std::stoi(argv[++i]));
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -s, --server ADDR - Specify server address (default: 127.0.0.1)" << std::endl;
            std::cout << "  -p, --port PORT   - Specify port (default: " << DEFAULT_PORT << ")" << std::endl;
            std::cout << "  -h, --help        - Show this help message" << std::endl;
            return 0;
        }
    }

    std::cout << "Connecting to server at " << server_address << ":" << server_port << std::endl;

    try
    {
        // Create client
        Client client(server_address, server_port);

        // Enable auto-reconnect
        client.setAutoReconnect(true, 30); // Try reconnect every 30 seconds

        // Connect to server
        if (!client.connect())
        {
            std::cerr << "Failed to connect to server" << std::endl;
            // We'll continue anyway and rely on auto-reconnect
        }
        else
        {
            // Send system info
            client.sendSystemInfo();
        }

        // Start client (start the command handler thread)
        client.start();

        // Main loop
        while (running)
        {
            // Just keep the program running and let the command thread do the work
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Stop client
        client.stop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}