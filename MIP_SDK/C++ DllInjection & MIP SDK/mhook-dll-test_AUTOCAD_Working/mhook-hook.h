#pragma once

//=========================================================================
// Define _NtClose so we can dynamically bind to the function
typedef BOOL(WINAPI* _CloseHandle)(IN HANDLE Handle);
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
typedef DWORD(WINAPI* _SetFilePointer)(
    HANDLE hFile,
    LONG   lDistanceToMove,
    PLONG  lpDistanceToMoveHigh,
    DWORD  dwMoveMethod
    );
typedef BOOL(WINAPI* _SetFilePointerEx)(
    HANDLE hFile,
    LARGE_INTEGER   lDistanceToMove,
    PLARGE_INTEGER  lpDistanceToMoveHigh,
    DWORD  dwMoveMethod
    );
typedef BOOL(WINAPI* _GetFileAttributesExW)(
    LPCWSTR                lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID                 lpFileInformation
    );
typedef DWORD(WINAPI* _GetFileSize)(HANDLE hFile,
    LPDWORD lpFileSizeHigh
    );
typedef BOOL(WINAPI* _GetFileSizeEx)(HANDLE hFile,
    PLARGE_INTEGER lpFileSize
    );
extern _CloseHandle TrueCloseHandle;
extern _CreateFileW TrueCreateFileW;
extern _ReadFile TrueReadFile;
extern _SetFilePointer TrueSetFilePointer;
extern _SetFilePointerEx TrueSetFilePointerEx;
extern _GetFileAttributesExW TrueGetFileAttributesExW;
extern _GetFileSize TrueGetFileSize;
extern _GetFileSizeEx TrueGetFileSizeEx;

BOOL WINAPI HookCloseHandle(HANDLE hHandle);

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
    LPOVERLAPPED lpOverlapped);
DWORD WINAPI HookSetFilePointer(
    HANDLE hFile,
    LONG   lDistanceToMove,
    PLONG  lpDistanceToMoveHigh,
    DWORD  dwMoveMethod);
BOOL WINAPI HookSetFilePointerEx(
    HANDLE hFile,
    LARGE_INTEGER   lDistanceToMove,
    PLARGE_INTEGER  lpDistanceToMoveHigh,
    DWORD  dwMoveMethod
    );
BOOL WINAPI HookGetFileAttributesExW(
    LPCWSTR                lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID                 lpFileInformation);
DWORD WINAPI HookGetFileSize(HANDLE hFile,
    LPDWORD lpFileSizeHigh
    );


namespace MYMIP
{

    void MIPInit();
    BOOL MIPReadProtectedFile(LPVOID lpBuffer,
        DWORD        nNumberOfBytesToRead,
        LPDWORD      lpNumberOfBytesRead);
    DWORD MIPSetFilePointerProtected(LONG lDistanceToMove,
        PLONG lpDistanceToMoveHigh,
        DWORD dwMoveMethod);
    BOOL MIPSetFilePointerExProtected(LARGE_INTEGER  liDistanceToMove,
        PLARGE_INTEGER lpNewFilePointer,
        DWORD          dwMoveMethod);
    DWORD MIPGetFileAttributesExW();
    BOOL MIPCloseHandle();
}