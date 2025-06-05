#pragma once

//=========================================================================
// Define _NtClose so we can dynamically bind to the function
typedef ULONG(WINAPI* _CloseHandle)(IN HANDLE Handle);
typedef HANDLE(WINAPI* _CreateFileW)(LPCWSTR lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile);
typedef BOOL(WINAPI* _ReadFile)(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
);

extern _CloseHandle TrueCloseHandle;
extern _CreateFileW TrueCreateFileW;
extern _ReadFile TrueReadFile;

ULONG WINAPI HookCloseHandle(HANDLE hHandle);

HANDLE WINAPI HookCreateFileW(LPCWSTR lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile);

BOOL WINAPI HookReadFile(HANDLE hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
);

namespace MYMIP
{

    void MIPInit();
    BOOL MIPReadProtectedFile(LPVOID lpBuffer,
        DWORD        nNumberOfBytesToRead,
        LPDWORD      lpNumberOfBytesRead);
}