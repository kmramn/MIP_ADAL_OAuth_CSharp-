
//Copyright (c) 2007-2008, Marton Anka
//
//Permission is hereby granted, free of charge, to any person obtaining a 
//copy of this software and associated documentation files (the "Software"), 
//to deal in the Software without restriction, including without limitation 
//the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//and/or sell copies of the Software, and to permit persons to whom the 
//Software is furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included 
//in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
//OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
//THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//IN THE SOFTWARE.

#include "stdafx.h"

//=========================================================================
// Get the current (original) address to the functions to be hooked
//
_CloseHandle TrueCloseHandle = (_CloseHandle)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CloseHandle");
_CreateFileW TrueCreateFileW = (_CreateFileW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateFileW");
_ReadFile TrueReadFile = (_ReadFile)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "ReadFile");
_SetFilePointer TrueSetFilePointer = (_SetFilePointer)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetFilePointer");
_SetFilePointerEx TrueSetFilePointerEx = (_SetFilePointerEx)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetFilePointerEx");
_GetFileAttributesExW TrueGetFileAttributesExW = (_GetFileAttributesExW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetFileAttributesExW");
_GetFileSize TrueGetFileSize = (_GetFileSize)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetFileSize");


//=========================================================================
// This is the function that will replace NtClose once the hook 
// is in place
//
HANDLE WINAPI HookCreateFileW(LPCWSTR lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile)
{
    //printf("***** Call to NtClose(0x%p)\n", hHandle);
    //WCHAR wszDBuf[1024];
    //SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
    //swprintf(wszDBuf, L"HookCreateFileW lpFileName-%s", lpFileName);
    //OutputDebugString(wszDBuf);
    if (0 == lstrcmpiW(lpFileName, L"D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt"))
    {
        MessageBox(NULL, lpFileName, L"mhook-dll-test", MB_OK);
    }
    return TrueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


/*BOOL GetFileNameFromHandle(HANDLE hFile, wstring& pszFileName)
{
    pszFileName = L"";

    unsigned int nSize = (_MAX_PATH + 1) * sizeof(TCHAR) + sizeof(FILE_NAME_INFO);
    //std::unique_ptr<BYTE[]> ptrcFni(new BYTE[nSize]);
    //SecureZeroMemory(ptrcFni.get(), nSize);
    //FILE_NAME_INFO* pFni = reinterpret_cast<FILE_NAME_INFO*>(ptrcFni.get());
    std::vector<BYTE> buffer(nSize, 0);
    FILE_NAME_INFO* pFni = reinterpret_cast<FILE_NAME_INFO*>(buffer.data());
    //SecureZeroMemory(pFni, nSize);

    //BOOL b = GetFileInformationByHandleEx(hFile, FileNameInfo, pFni, sizeof(FILE_NAME_INFO) + (_MAX_PATH * sizeof(TCHAR)));
    BOOL b = GetFileInformationByHandleEx(hFile, FileNameInfo, pFni, nSize);
    DWORD dw = GetLastError();
    if (dw==0 && b)
    {
        //pFni->FileName[pFni->FileNameLength + 1] = L'\0';
        pFni->FileName[pFni->FileNameLength] = L'\0';
        pszFileName = pFni->FileName;
    }
    return b;
}*/

BOOL GetFileNameFromHandle(HANDLE hFile, wstring& pszFileName)
{
    pszFileName = L"";

    unsigned int nSize = (_MAX_PATH + 1) * sizeof(TCHAR) + sizeof(FILE_NAME_INFO);
    std::vector<BYTE> buffer(2048, 0);
    FILE_NAME_INFO* pFni = reinterpret_cast<FILE_NAME_INFO*>(buffer.data());

    BOOL b = GetFileInformationByHandleEx(hFile, FileNameInfo, pFni, nSize);
    DWORD dw = GetLastError();
    if (dw == 0 && b)
    {
        pFni->FileName[pFni->FileNameLength + 1] = L'\0';
        pszFileName = pFni->FileName;
    }
    return b;
}

ApplicationInfo appInfo;
shared_ptr<MipConfiguration> mipConfiguration;
shared_ptr<MipContext> mipContext;
shared_ptr<FileProfile> fileProfile;
shared_ptr<FileEngine> fileEngine;
shared_ptr<FileHandler> fileHandler;
std::vector<uint8_t> fileData;
shared_ptr<ProtectionHandler> fileProtectionHandler;
filebuf fb;
shared_ptr<istream> ioStream = nullptr;
shared_ptr<mip::Stream> iStream = nullptr;
shared_ptr<mip::Stream> protectedStream = nullptr;

thread_local bool gHook = false;

BOOL WINAPI HookReadFile(HANDLE hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
)
{
    BOOL bRet = TRUE;
    if (false == gHook)
    {
        gHook = true;
        wstring wszFileName = L"";
        GetFileNameFromHandle(hFile, wszFileName);
        if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", 51) == 0))
        {
            OutputDebugStringW(L"HookReadFile");
            OutputDebugString(wszFileName.c_str());
            //bRet = TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
            //static bool bMIPInit = false;
            //if (false == bMIPInit)
            //if (nullptr == protectedStream)
            //{
            //    bMIPInit = true;
            //    MIPInit();
            //}
            WCHAR wszDBuf[1024];
            SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
            swprintf(wszDBuf, L"nNumberOfBytesToRead-%u", nNumberOfBytesToRead);
            OutputDebugString(wszDBuf);

            DWORD dwLastError = 0;
            BOOL bRet = MIPReadProtectedFile(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, dwLastError);
            if (!bRet)
            {
                SetLastError(dwLastError);
            }
            else {
                if (NULL != lpNumberOfBytesRead)
                    swprintf(wszDBuf, L"nNumberOfBytesToRead-%u, lpNumberOfBytesRead-%u", nNumberOfBytesToRead, *lpNumberOfBytesRead);
                else
                    swprintf(wszDBuf, L"nNumberOfBytesToRead-%u", nNumberOfBytesToRead);
                OutputDebugString(wszDBuf);
                OutputDebugStringA((LPCSTR)lpBuffer);
            }
        }
        else
        {
            bRet = TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
            if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 53) == 0))
            {
                OutputDebugStringW(L"HookReadFile");
                OutputDebugString(wszFileName.c_str());
                WCHAR wszDBuf[1024];
                SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
                if (NULL != lpNumberOfBytesRead)
                    swprintf(wszDBuf, L"nNumberOfBytesToRead-%u, lpNumberOfBytesRead-%u", nNumberOfBytesToRead, *lpNumberOfBytesRead);
                else
                    swprintf(wszDBuf, L"nNumberOfBytesToRead-%u", nNumberOfBytesToRead);
                OutputDebugString(wszDBuf);
                OutputDebugStringA((LPCSTR)lpBuffer);
            }
        }
        gHook = false;
    }
    else
        bRet = TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

    return bRet;
}

DWORD WINAPI HookSetFilePointer(HANDLE hFile,
    LONG   lDistanceToMove,
    PLONG  lpDistanceToMoveHigh,
    DWORD  dwMoveMethod
)
{
    DWORD dwRet = INVALID_SET_FILE_POINTER;
    if (false == gHook)
    {
        gHook = true;
        wstring wszFileName = L"";
        GetFileNameFromHandle(hFile, wszFileName);
        if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", 51) == 0))
        {
            OutputDebugString(L"HookSetFilePointer()");
            OutputDebugString(wszFileName.c_str());
            //MessageBox(NULL, L"HookSetFilePointer", L"1", MB_OK);
            //dwRet = TrueSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
            WCHAR wszDBuf[1024];
            SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
            if (NULL != lpDistanceToMoveHigh)
                swprintf(wszDBuf, L"B lDistanceToMove-%d, lpDistanceToMoveHigh-%d, dwMoveMethod-%d", lDistanceToMove, *lpDistanceToMoveHigh, dwMoveMethod);
            else
                swprintf(wszDBuf, L"B lDistanceToMove-%d, dwMoveMethod-%d", lDistanceToMove, dwMoveMethod);
            OutputDebugString(wszDBuf);
            dwRet = MIPSetFilePointerProtected(lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
            SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
            if (NULL != lpDistanceToMoveHigh)
                swprintf(wszDBuf, L"A lDistanceToMove-%d, lpDistanceToMoveHigh-%d, dwMoveMethod-%d", lDistanceToMove, *lpDistanceToMoveHigh, dwMoveMethod);
            else
                swprintf(wszDBuf, L"A lDistanceToMove-%d, dwMoveMethod-%d", lDistanceToMove, dwMoveMethod);
            OutputDebugString(wszDBuf);
            //dwRet = MIPSetFilePointerProtected(lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
        }
        else
        {
            dwRet = TrueSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
            if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 53) == 0))
            {
                OutputDebugString(L"HookSetFilePointer()");
                OutputDebugString(wszFileName.c_str());
                WCHAR wszDBuf[1024];
                SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
                if (NULL != lpDistanceToMoveHigh)
                    swprintf(wszDBuf, L"B lDistanceToMove-%d, lpDistanceToMoveHigh-%d, dwMoveMethod-%d", lDistanceToMove, *lpDistanceToMoveHigh, dwMoveMethod);
                else
                    swprintf(wszDBuf, L"B lDistanceToMove-%d, dwMoveMethod-%d", lDistanceToMove, dwMoveMethod);
                OutputDebugString(wszDBuf);
            }
        }
        gHook = false;
    }
    else
        dwRet = TrueSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);

    return dwRet;
}

BOOL WINAPI HookSetFilePointerEx(HANDLE hFile,
    LARGE_INTEGER lDistanceToMove,
    PLARGE_INTEGER lpDistanceToMoveHigh,
    DWORD  dwMoveMethod
)
{
    BOOL bRet = TRUE;
    if (false == gHook)
    {
        gHook = true;
        wstring wszFileName = L"";
        GetFileNameFromHandle(hFile, wszFileName);
        if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", 51) == 0))
        {
            OutputDebugString(L"HookSetFilePointerEx()");
            OutputDebugString(wszFileName.c_str());
            //MessageBox(NULL, L"HookSetFilePointerEx", L"1", MB_OK);
            //bRet = TrueSetFilePointerEx(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
            bRet = MIPSetFilePointerExProtected(lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
            WCHAR wszDBuf[1024];
            SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
            if (NULL != lpDistanceToMoveHigh)
                swprintf(wszDBuf, L"lDistanceToMove-%lld, lpDistanceToMoveHigh-%lld, dwMoveMethod-%d",
                    lDistanceToMove.QuadPart, (*lpDistanceToMoveHigh).QuadPart, dwMoveMethod);
            else
                swprintf(wszDBuf, L"lDistanceToMove-%lld, dwMoveMethod-%d", lDistanceToMove.QuadPart, dwMoveMethod);
            OutputDebugString(wszDBuf);
        }
        else
        {
            bRet = TrueSetFilePointerEx(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
            if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 53) == 0))
            {
                OutputDebugString(L"HookSetFilePointerEx()");
                OutputDebugString(wszFileName.c_str());
                WCHAR wszDBuf[1024];
                SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
                if (NULL != lpDistanceToMoveHigh)
                    swprintf(wszDBuf, L"lDistanceToMove-%lld, lpDistanceToMoveHigh-%lld, dwMoveMethod-%d",
                        lDistanceToMove.QuadPart, (*lpDistanceToMoveHigh).QuadPart, dwMoveMethod);
                else
                    swprintf(wszDBuf, L"lDistanceToMove-%lld, dwMoveMethod-%d", lDistanceToMove.QuadPart, dwMoveMethod);
                OutputDebugString(wszDBuf);
            }
        }
        gHook = false;
    }
    else
        bRet = TrueSetFilePointerEx(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);

    return bRet;
}

BOOL WINAPI HookGetFileAttributesExW(LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
)
{
    BOOL bRet = TRUE;
    if (false == gHook)
    {
        gHook = true;
        //if (StrCmpNIW(lpFileName, L"D:\\Test\\Test\\CPPMIPSDKTestFiles\\Protected\\Support.SLDPRT", 56) == 0)
        if ((StrCmpNIW(lpFileName, L"D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", 53) == 0))
        {
            //MessageBox(NULL, L"HookGetFileAttributesExW", L"1", MB_OK);
            static bool bMIPInit = false;
            if (false == bMIPInit)
            //if (nullptr == protectedStream)
            {
                bMIPInit = true;
                MIPInit();
            }
            OutputDebugString(L"HookGetFileAttributesExW()");
            OutputDebugString(lpFileName);
            DWORD dwSize = MIPGetFileAttributesExW();
            //static int i = 0;
            //if (i <= 3)
            //{
            //    i++;
            //    wchar_t wszBuff[256];
            //    SecureZeroMemory(wszBuff, 512);
            //    swprintf_s(wszBuff, 256, L"Size = %u", dwSize);
            //    MessageBox(NULL, wszBuff, L"2", MB_OK);
            //}
            if (dwSize > 0)
            {
                bRet = TrueGetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);
                LARGE_INTEGER ln1;
                ln1.QuadPart = dwSize;
                ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->nFileSizeLow = ln1.LowPart;
                ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->nFileSizeHigh = ln1.HighPart;
                WCHAR wszDBuf[1024];
                SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
                swprintf(wszDBuf, L"dwFileAttributes-%u, nFileSizeLow-%u, nFileSizeHigh-%d", ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->dwFileAttributes, ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->nFileSizeLow, ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->nFileSizeHigh);
                OutputDebugString(wszDBuf);
            }
        }
        else
        {
            if ((StrCmpNIW(lpFileName, L"D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 55) == 0))
            {
                OutputDebugString(L"HookGetFileAttributesExW()");
                OutputDebugString(lpFileName);
            }
            bRet = TrueGetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);
            if ((StrCmpNIW(lpFileName, L"D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 55) == 0))
            {
                WCHAR wszDBuf[1024];
                SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
                swprintf(wszDBuf, L"dwFileAttributes-%u, nFileSizeLow-%u, nFileSizeHigh-%d", ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->dwFileAttributes, ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->nFileSizeLow, ((WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation)->nFileSizeHigh);
                OutputDebugString(wszDBuf);
            }
        }
        gHook = false;
    }
    else
        bRet = TrueGetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);

    return bRet;
}

DWORD WINAPI HookGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    DWORD dwRet = TRUE;
    if (false == gHook)
    {
        gHook = true;
        wstring wszFileName = L"";
        GetFileNameFromHandle(hFile, wszFileName);
        if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", 51) == 0))
        {
            OutputDebugString(L"HookGetFileSize");
            OutputDebugString(wszFileName.c_str());
            //MessageBox(NULL, L"HookGetFileSize", L"1", MB_OK);
            dwRet = MIPGetFileAttributesExW();
            if (NULL != lpFileSizeHigh)
                *lpFileSizeHigh = 0;
        }
        else
        {
            if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 53) == 0))
            {
                OutputDebugString(L"HookGetFileSize");
                OutputDebugString(wszFileName.c_str());
            }
            dwRet = TrueGetFileSize(hFile, lpFileSizeHigh);
        }
        gHook = false;
    }
    else
        dwRet = TrueGetFileSize(hFile, lpFileSizeHigh);

    return dwRet;
}

BOOL WINAPI HookCloseHandle(HANDLE hHandle)
{
    BOOL bRet = TRUE;
    if (false == gHook)
    {
        gHook = true;
        wstring wszFileName = L"";
        GetFileNameFromHandle(hHandle, wszFileName);
        if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", 51) == 0))
        {
            OutputDebugString(L"HookCloseHandle");
            OutputDebugString(wszFileName.c_str());
            //MessageBox(NULL, L"HookGetFileSize", L"1", MB_OK);
            bRet = TrueCloseHandle(hHandle);
            bRet = MIPCloseHandle();
        }
        else
        {
            if ((StrCmpNIW(wszFileName.c_str(), L"\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_unprotected.jt", 53) == 0))
            {
                OutputDebugString(L"HookCloseHandle");
                OutputDebugString(wszFileName.c_str());
            }
            bRet = TrueCloseHandle(hHandle);
        }
        gHook = false;
    }
    else
        bRet = TrueCloseHandle(hHandle);

    return bRet;
}

namespace MYMIP
{
    void MIPInit()
    {
        //MessageBox(NULL, L"Slep Before", L"1", MB_OK);
        Sleep(10000);
        try
        {
            //MessageBox(NULL, L"MIPInit", L"1", MB_OK);
            //MessageBox(NULL, L"MIPInit", L"1.1", MB_OK);
            appInfo.applicationId = "9ed1f576-dada-42f4-aa49-877b2e71bedd";
            //MessageBox(NULL, L"MIPInit", L"1.2", MB_OK);
            appInfo.applicationName = "HALOCADApp";
            //MessageBox(NULL, L"MIPInit", L"1.3", MB_OK);
            appInfo.applicationVersion = "1.0.0.0";
            //MessageBox(NULL, L"MIPInit", L"1.4", MB_OK);
#ifdef _DEBUG
            shared_ptr<MipConfiguration> mipConfiguration = make_shared<MipConfiguration>(appInfo, "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Debug\\mip_log", LogLevel::Error, false);
#elif NDEBUG
            mipConfiguration = make_shared<MipConfiguration>(appInfo, "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\x64\\Release\\mip_log", LogLevel::Error, false);
#endif
            //MessageBox(NULL, L"MIPInit", L"1.5", MB_OK);
            // Create Context
            mipContext = MipContext::Create(mipConfiguration);
            //MessageBox(NULL, L"MIPInit", L"2", MB_OK);

            // Create Profile
            shared_ptr<ConsentDelegate> consentDelegate = make_shared<ConsentDelegateImpl>(false);
            const shared_ptr<ProfileObserver> sampleProfileObserver = make_shared<ProfileObserver>();
            FileProfile::Settings fileProfileSettings(mipContext, CacheStorageType::OnDisk, consentDelegate, sampleProfileObserver);
            shared_ptr<std::promise<shared_ptr<FileProfile>>> loadPromise = make_shared<std::promise<shared_ptr<FileProfile>>>();
            future<shared_ptr<FileProfile>> loadFuture = loadPromise->get_future();
            FileProfile::LoadAsync(fileProfileSettings, loadPromise); // Getting the profile
            fileProfile = loadFuture.get();
            //MessageBox(NULL, L"MIPInit", L"3", MB_OK);

            // Create fileengine
            string sccToken = "";
            string protectionToken = "";
            shared_ptr<AuthDelegateImpl> authDelegate = make_shared<AuthDelegateImpl>(false /*isVerbose*/, "", "", "", sccToken, protectionToken, ".");
            FileEngine::Settings fileeEngineSettings(Identity("Cadauthor@halosecude.onmicrosoft.com"), authDelegate, "");
            fileeEngineSettings.SetCloud(mip::Cloud::Commercial);
            auto addEnginePromise = make_shared<std::promise<shared_ptr<FileEngine>>>();
            auto addEngineFuture = addEnginePromise->get_future();
            fileProfile->AddEngineAsync(fileeEngineSettings, addEnginePromise); // Getting the engine
            fileEngine = addEngineFuture.get();
            //MessageBox(NULL, L"MIPInit", L"4", MB_OK);

            // Create File handler
            string inputFilePath = "D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt";
            string actualFilePath = "D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt";
            const shared_ptr<FileHandlerObserver> sampleFileHandlerObserver = make_shared<FileHandlerObserver>();
            shared_ptr<FileExecutionStateImpl> fileExecutionState = make_shared<FileExecutionStateImpl>(DataState::REST, nullptr, false, "");
            shared_ptr<promise<shared_ptr<FileHandler>>> createFileHandlerPromise = make_shared<std::promise<shared_ptr<FileHandler>>>();
            future<shared_ptr<FileHandler>> createFileHandlerFuture = createFileHandlerPromise->get_future();
            //fileEngine->CreateFileHandlerAsync(
            //    inputFilePath, actualFilePath, false, sampleFileHandlerObserver, createFileHandlerPromise, fileExecutionState);

            //// open the file:
            std::streampos fileSize;
            std::ifstream file(inputFilePath, std::ios::in | std::ios::binary);
            // get its size:
            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            // read the data:
            fileData.resize(fileSize);
            file.read((char*)&fileData[0], fileSize);
            shared_ptr<Stream> fhStream = CreateStreamFromBuffer(fileData.data(), fileData.size());
            fileEngine->CreateFileHandlerAsync(
                fhStream, actualFilePath, false, sampleFileHandlerObserver, createFileHandlerPromise, fileExecutionState);

            fileHandler = createFileHandlerFuture.get();
            //MessageBox(NULL, L"MIPInit", L"5", MB_OK);

            fileProtectionHandler = fileHandler->GetProtection();
            if (nullptr == fileProtectionHandler)
                MessageBox(NULL, L"6", L"8", MB_OK);

            //filebuf fb;
            fb.open("D:\\FromE\\Daniel\\VC++\\MIPTest\\MODEL$Unpro_protected.jt", ios::in | ios::binary);
            //shared_ptr<istream> ioStream = make_shared<istream>(&fb);
            //shared_ptr<Stream> iStream = CreateStreamFromStdStream(ioStream);
            ioStream = make_shared<istream>(&fb);
            iStream = CreateStreamFromStdStream(ioStream);
            if (nullptr != fileProtectionHandler)
                //shared_ptr<Stream> protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x2ccf, 0x1180);
                //protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x2ccf, 0x19180); // Old Support.SLDPART giving privilaged error
                protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x87FA, 0x2D80);
            //protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x5847, 0x10);
            //uint8_t unData[3];
            //protectedStream->Seek(1);
            //protectedStream->Read(unData, 3);
            MessageBox(NULL, L"MIPInit", L"9", MB_OK);
        }
        catch (const cxxopts::OptionException& ex) {
            cout << "Error parsing options: " << ex.what() << endl;
            OutputDebugStringA(ex.what());
            return;
        }
        catch (const std::exception& ex) {
            cout << "Something bad happened: " << ex.what() << "\nExiting." << endl;
            OutputDebugStringA("**********************************");
            OutputDebugStringA(ex.what());
            OutputDebugStringA("**********************************");
            return;
        }

    }

    BOOL MIPReadProtectedFile(LPVOID lpBuffer,
        DWORD        nNumberOfBytesToRead,
        LPDWORD      lpNumberOfBytesRead,
        DWORD& dwLastError)
    {
        if (nullptr != protectedStream)
        {
            LONGLONG llCurrentPosition = protectedStream->Position();
            DWORD nNumberOfBytesCanRead = 11637 - llCurrentPosition;
            if (nNumberOfBytesToRead > nNumberOfBytesCanRead)
            {
                dwLastError = ERROR_HANDLE_EOF;
                return FALSE;
            }
            *lpNumberOfBytesRead = protectedStream->Read((uint8_t*)lpBuffer, nNumberOfBytesToRead);
        }
        return TRUE;
    }

    DWORD MIPSetFilePointerProtected(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
    {
        if (nullptr != protectedStream)
        {
            WCHAR wszDBuf[1024];
            SecureZeroMemory(wszDBuf, sizeof(wszDBuf));
            if (FILE_CURRENT == dwMoveMethod)
            {
                LARGE_INTEGER ln1;
                ln1.LowPart = lDistanceToMove;
                if (NULL != lpDistanceToMoveHigh)
                    ln1.HighPart = *lpDistanceToMoveHigh;
                else ln1.HighPart = 0;
                swprintf(wszDBuf, L"B lDistanceToMove-%d", ln1.QuadPart);
                OutputDebugString(L"MIPSetFilePointerProtected()");
                OutputDebugString(wszDBuf);
                if (0 < ln1.QuadPart)
                    protectedStream->Seek(ln1.QuadPart);
                else
                    protectedStream->Seek(11637 - ln1.QuadPart);
                ln1.QuadPart = protectedStream->Position();
                lDistanceToMove = ln1.LowPart;
                if (NULL != lpDistanceToMoveHigh) *lpDistanceToMoveHigh = ln1.HighPart;
            }
            else if (FILE_BEGIN == dwMoveMethod)
            {
                protectedStream->Seek(0);
                LARGE_INTEGER ln1;
                ln1.LowPart = lDistanceToMove;
                if (NULL != lpDistanceToMoveHigh)
                    ln1.HighPart = *lpDistanceToMoveHigh;
                else ln1.HighPart = 0;
                swprintf(wszDBuf, L"B lDistanceToMove-%d", ln1.QuadPart);
                OutputDebugString(L"MIPSetFilePointerProtected()");
                OutputDebugString(wszDBuf);
                protectedStream->Seek(ln1.QuadPart);
                ln1.QuadPart = protectedStream->Position();
                lDistanceToMove = ln1.LowPart;
                if (NULL != lpDistanceToMoveHigh) *lpDistanceToMoveHigh = ln1.HighPart;
            }
            else if (FILE_END == dwMoveMethod)
            {
                protectedStream->Seek(0);
                LARGE_INTEGER ln1;
                ln1.LowPart = lDistanceToMove;
                if (NULL != lpDistanceToMoveHigh)
                    ln1.HighPart = *lpDistanceToMoveHigh;
                else ln1.HighPart = 0;
                swprintf(wszDBuf, L"B lDistanceToMove-%d", ln1.QuadPart);
                OutputDebugString(L"MIPSetFilePointerProtected()");
                OutputDebugString(wszDBuf);
                protectedStream->Seek(11637 - ln1.QuadPart);
                ln1.QuadPart = protectedStream->Position();
                lDistanceToMove = ln1.LowPart;
                if (NULL != lpDistanceToMoveHigh) *lpDistanceToMoveHigh = ln1.HighPart;
            }
        }
        return lDistanceToMove;
    }

    BOOL MIPSetFilePointerExProtected(LARGE_INTEGER  liDistanceToMove,
        PLARGE_INTEGER lpNewFilePointer,
        DWORD          dwMoveMethod
    )
    {
        if (nullptr != protectedStream)
        {
            if (FILE_BEGIN == dwMoveMethod)
            {
                protectedStream->Seek(0);
                protectedStream->Seek(liDistanceToMove.QuadPart);
                if (NULL != lpNewFilePointer) (*lpNewFilePointer).QuadPart = protectedStream->Position();
            }
            else if (FILE_CURRENT == dwMoveMethod)
            {
                LONGLONG llCurrentPosition = protectedStream->Position();
                protectedStream->Seek(llCurrentPosition + liDistanceToMove.QuadPart);
                if (NULL != lpNewFilePointer) (*lpNewFilePointer).QuadPart = protectedStream->Position();
            }
            else if (FILE_END == dwMoveMethod)
            {
                protectedStream->Seek(0);
                protectedStream->Seek(11637 - liDistanceToMove.QuadPart);
                if (NULL != lpNewFilePointer) (*lpNewFilePointer).QuadPart = protectedStream->Position();
            }
        }
        return TRUE;
    }

    DWORD MIPGetFileAttributesExW()
    {
        //if (nullptr != protectedStream)
        //    return  protectedStream->Size();
        //else return 0;
        return 11637;
    }

    BOOL MIPCloseHandle()
    {
        if (nullptr != protectedStream)
            protectedStream->Seek(0);
        return TRUE;
    }
}
