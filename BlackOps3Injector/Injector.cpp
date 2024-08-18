#include "Injector.h"

int main()
{
    std::string dllPath = GetDLLPath("BlackOps3Cheats.dll");

    HANDLE processHandle = GetProcessHandle(L"BlackOps3.exe", false);
    std::cout << processHandle << '\n';

    //WriteProcessMemory(processHandle, ExitProcess, (BYTE*)"\xB8\x0\x0\x0\x0\xC3", 6, nullptr); // return 0
    //WriteProcessMemory(processHandle, VirtualQueryEx, (BYTE*)"\xB8\x0\x0\x0\x0\xC3", 6, nullptr); // return 0

    bool succeeded = false;

    if (processHandle && processHandle != INVALID_HANDLE_VALUE)
    {
        succeeded = InjectByLoadLibraryA(processHandle, dllPath.c_str());

        CloseHandle(processHandle);
    }
    else 
    {
        std::cout << "Failed to get a handle to the process\n";
        std::cin.get();
    }

    if(!succeeded)
    {
        std::cout << "Failed to inject DLL\n";
        std::cin.get();
    }

    return 0;
}

HANDLE GetProcessHandle(const wchar_t* procName, bool useDebugPrivilege)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, procName))
                {
                    CloseHandle(hSnap);

                    if (useDebugPrivilege)
                    {
                        LUID luid;
                        LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &luid);

                        TOKEN_PRIVILEGES tp;
                        tp.PrivilegeCount = 1;
                        tp.Privileges[0].Luid = luid;
                        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                        HANDLE accessToken;
                        OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &accessToken);

                        AdjustTokenPrivileges(accessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
                    }

                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry.th32ProcessID);
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }

    CloseHandle(hSnap);
    return INVALID_HANDLE_VALUE;
}

std::string GetDLLPath(const char* dllName)
{
    char path[MAX_PATH];

    GetModuleFileNameA(NULL, path, MAX_PATH);

    std::string str(path);

    str = str.substr(0, str.find_last_of("\\") + 1);
    str += std::string(dllName);

    return str;
}

bool InjectByLoadLibraryA(HANDLE procHandle, const char* dllPath)
{
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "DLL file not found\n";
        return false;
    }

    void* dllPathLocation = VirtualAllocEx(procHandle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!dllPathLocation)
    {
        std::cout << "Failed to allocate memory for dll path\n";
        return false;
    }

    WriteProcessMemory(procHandle, dllPathLocation, dllPath, strlen(dllPath) + 1, 0);

    HANDLE hThread = CreateRemoteThread(procHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, dllPathLocation, 0, 0);

    if (hThread && hThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hThread);
    }
    else
    {
        std::cout << "Failed to create remote thread in process\n";
        return false;
    }

    return true;
}