
#include "stdafx.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
        SetDllDirectory(L"D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Debug");
#elif NDEBUG
        SetDllDirectory(L"D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Release");
#endif
        MessageBox(NULL, L"1", L"2", MB_OK);
        //MIPInit();
        MessageBox(NULL, L"3", L"4", MB_OK);
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        //printf("Testing Close.\n");
        //if (Mhook_SetHook((PVOID*)&TrueCreateFileW, (PVOID)HookCreateFileW))
        //{
            //CloseHandle(NULL);
            // Remove the hook
            //Mhook_Unhook((PVOID*)&TrueCloseHandle);
        //}

        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        Mhook_SetHook((PVOID*)&TrueReadFile, (PVOID)HookReadFile);
        Mhook_SetHook((PVOID*)&TrueSetFilePointer, (PVOID)HookSetFilePointer);
        //Mhook_SetHook((PVOID*)&TrueSetFilePointerEx, (PVOID)HookSetFilePointerEx);
        Mhook_SetHook((PVOID*)&TrueGetFileAttributesExW, (PVOID)HookGetFileAttributesExW);
        //Mhook_SetHook((PVOID*)&TrueGetFileSize, (PVOID)HookGetFileSize);
        Mhook_SetHook((PVOID*)&TrueCloseHandle, (PVOID)HookCloseHandle);

        break;

    case DLL_THREAD_ATTACH:
        //MIPInit();
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:

        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}