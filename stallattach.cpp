#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cassert>
#define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "debugapi.h"
#include "privilege.h"

using namespace std::chrono_literals;

// Taken from: https://www.codeproject.com/Tips/479880/GetLastError-as-std-string
std::string GetLastErrorStdStr()
{
    DWORD error = GetLastError();
    if (error)
    {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, NULL);
        if (bufLen)
        {
            LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
            // BUG: does not convert correctly from utf16
            std::string result(lpMsgStr, lpMsgStr + bufLen);

            LocalFree(lpMsgBuf);

            return result;
        }
    }
    return std::string();
}

static void raise_privileges()
{
    BOOL isOK;
    HANDLE hToken;
    HANDLE hCurrentProcess;
    hCurrentProcess = GetCurrentProcess();
    isOK = OpenProcessToken(hCurrentProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
    assert(isOK);
    SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Expected PID argument\n";
        return 1;
    }
    const unsigned pid = std::atoi(argv[1]);
    std::cout << "Attaching to PID " << pid << "\n";

    raise_privileges();

    bool success = DebugActiveProcess(pid);
    if (!success) {
        std::string explanation = GetLastErrorStdStr();
        std::cerr << "Cannot attach to application: " << explanation << "\n";
        return 1;
    }

    std::cout << "Stalling the application forever\n";
    while (true) {
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
