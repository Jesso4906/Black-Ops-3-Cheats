#pragma once
#include <iostream>
#include <fstream>
#include <windows.h>
#include <TlHelp32.h>

HANDLE GetProcessHandle(const wchar_t* procName, bool useDebugPrivilege);

std::string GetDLLPath(const char* dllName);

bool InjectByLoadLibraryA(HANDLE procHandle, const char* dllPath);