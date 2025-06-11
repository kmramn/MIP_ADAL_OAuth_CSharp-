
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


//=========================================================================
// This is the function that will replace NtClose once the hook 
// is in place
//
ULONG WINAPI HookCloseHandle(HANDLE hHandle)
{
    //printf("***** Call to NtClose(0x%p)\n", hHandle);
    WCHAR wszFileName[MAX_PATH];
    SecureZeroMemory(wszFileName, MAX_PATH * sizeof(WCHAR));
    if (0 != GetFinalPathNameByHandle(hHandle, wszFileName, MAX_PATH, VOLUME_NAME_NT)) {
        MessageBox(NULL, wszFileName, L"mhook-dll-test", MB_OK);
    }
    return TrueCloseHandle(hHandle);
}

HANDLE WINAPI HookCreateFileW(LPCWSTR lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile)
{
    //printf("***** Call to NtClose(0x%p)\n", hHandle);
    if (0 == lstrcmpiW(lpFileName, L"D:\\Test\\Test\\CPPMIPSDKTestFiles\\UnProtected\\Support1.SLDPRT"))
    {
        MessageBox(NULL, lpFileName, L"mhook-dll-test", MB_OK);
    }
    return TrueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


BOOL GetFileNameFromHandle(HANDLE hFile, wstring& pszFileName)
{
    pszFileName = L"";

    std::unique_ptr<BYTE[]> ptrcFni(new BYTE[_MAX_PATH * sizeof(TCHAR) + sizeof(FILE_NAME_INFO)]);
    FILE_NAME_INFO* pFni = reinterpret_cast<FILE_NAME_INFO*>(ptrcFni.get());

    BOOL b = GetFileInformationByHandleEx(hFile, FileNameInfo,
        pFni, sizeof(FILE_NAME_INFO) + (_MAX_PATH * sizeof(TCHAR)));
    if (b)
    {
        pFni->FileName[pFni->FileNameLength + 1] = L'\0';
        pszFileName = pFni->FileName;
    }
    return b;
}

filebuf fb;
shared_ptr<istream> ioStream = nullptr;
shared_ptr<Stream> iStream = nullptr;
shared_ptr<Stream> protectedStream = nullptr;

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
        OutputDebugStringW(L"HookReadFile");
        wstring wszFileName = L"";
        GetFileNameFromHandle(hFile, wszFileName);
        OutputDebugString(wszFileName.c_str());
        if (StrCmpNIW(wszFileName.c_str(), L"\\Daniel\\VC++\\MIPTest\\Test.txt", 29) == 0)
        {
            static bool bMIPInit = false;
            if (false == bMIPInit)
            //if (nullptr == protectedStream)
            {
                bMIPInit = true;
                MIPInit();
            }
            MIPReadProtectedFile(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead);
        }
        else
        {
            bRet = TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        }
        gHook = false;
    }
    else
        bRet = TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

    return bRet;
}

namespace MYMIP
{
    void MIPInit()
    {
        MessageBox(NULL, L"MIPInit Sleep", L"1", MB_OK);
        Sleep(10000);
        try
        {
            MessageBox(NULL, L"MIPInit", L"1", MB_OK);
            ApplicationInfo appInfo;
            MessageBox(NULL, L"MIPInit", L"1.1", MB_OK);
            appInfo.applicationId = "9ed1f576-dada-42f4-aa49-877b2e71bedd";
            appInfo.applicationName = "HALOCADApp";
            appInfo.applicationVersion = "1.0.0.0";
            MessageBox(NULL, L"MIPInit", L"1.4", MB_OK);
#ifdef _DEBUG
            shared_ptr<MipConfiguration> mipConfiguration = make_shared<MipConfiguration>(appInfo, "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\mhook-dll-test\\x64\\Debug\\mip-data", LogLevel::Trace, false);
#elif NDEBUG
            shared_ptr<MipConfiguration> mipConfiguration = make_shared<MipConfiguration>(appInfo, "D:\\FromE\\Daniel\\VC++\\MIPTest\\mhook-dll-test\\mhook-dll-test\\x64\\Release\\mip-data", LogLevel::Warning, false);
#endif
            MessageBox(NULL, L"MIPInit", L"1.5", MB_OK);
            // Create Context
            shared_ptr<MipContext> mipContext = MipContext::Create(mipConfiguration);
            MessageBox(NULL, L"MIPInit", L"2", MB_OK);

            // Create Profile
            shared_ptr<ConsentDelegate> consentDelegate = make_shared<ConsentDelegateImpl>(false);
            const shared_ptr<ProfileObserver> sampleProfileObserver = make_shared<ProfileObserver>();
            FileProfile::Settings fileProfileSettings(mipContext, CacheStorageType::OnDisk, consentDelegate, sampleProfileObserver);
            shared_ptr<std::promise<shared_ptr<FileProfile>>> loadPromise = make_shared<std::promise<shared_ptr<FileProfile>>>();
            future<shared_ptr<FileProfile>> loadFuture = loadPromise->get_future();
            FileProfile::LoadAsync(fileProfileSettings, loadPromise); // Getting the profile
            shared_ptr<FileProfile> fileProfile = loadFuture.get();
            MessageBox(NULL, L"MIPInit", L"3", MB_OK);

            // Create fileengine
            string sccToken = "";
            string protectionToken = "";
            shared_ptr<AuthDelegateImpl> authDelegate = make_shared<AuthDelegateImpl>(false /*isVerbose*/, "", "", "", sccToken, protectionToken, ".");
            FileEngine::Settings fileeEngineSettings(Identity("Cadauthor@halocad.onmicrosoft.com"), authDelegate, "");
            fileeEngineSettings.SetCloud(mip::Cloud::Commercial);
            auto addEnginePromise = make_shared<std::promise<shared_ptr<FileEngine>>>();
            auto addEngineFuture = addEnginePromise->get_future();
            fileProfile->AddEngineAsync(fileeEngineSettings, addEnginePromise); // Getting the engine
            shared_ptr<FileEngine> fileEngine = addEngineFuture.get();
            MessageBox(NULL, L"MIPInit", L"4", MB_OK);

            // Create File handler
            string inputFilePath = "C:\\Daniel\\VC++\\MIPTest\\Test.txt";
            string actualFilePath = "C:\\Daniel\\VC++\\MIPTest\\Test.txt";

            //filebuf fb1;
            //fb1.open("C:\\Daniel\\VC++\\MIPTest\\Test.txt", ios::in | ios::binary);
            //fb1.open("C:\\Daniel\\VC++\\MIPTest\\Test.txt", std::fstream::in | std::fstream::out | std::fstream::ate | std::fstream::binary);
            //shared_ptr<istream> ioStream1 = make_shared<istream>(&fb1);
            //shared_ptr<Stream> iStream1 = CreateStreamFromStdStream(ioStream1);
            auto ioStream1 = std::make_shared<std::fstream>(
                "C:\\Daniel\\VC++\\MIPTest\\Test.txt", std::fstream::in | std::fstream::out | std::fstream::ate | std::fstream::binary);
            //auto iStream1 = mip::CreateStreamFromStdStream(std::static_pointer_cast<std::iostream>(ioStream1));
            std::vector<BYTE> header;
            header.resize(30897);
            ioStream1->seekg(0);
            ioStream1->read((char*)&header[0], 30897);
            auto iStream1 = mip::CreateStreamFromBuffer(&header[0], 30897);
            char szSoBuf[6];
            SecureZeroMemory(szSoBuf, 6);
            ioStream1->read(szSoBuf, 5);
            cout << szSoBuf << endl;
            ioStream1->close();

            const shared_ptr<FileHandlerObserver> sampleFileHandlerObserver = make_shared<FileHandlerObserver>();
            shared_ptr<FileExecutionStateImpl> fileExecutionState = make_shared<FileExecutionStateImpl>(DataState::REST, nullptr, false, "");
            shared_ptr<promise<shared_ptr<FileHandler>>> createFileHandlerPromise = make_shared<std::promise<shared_ptr<FileHandler>>>();
            future<shared_ptr<FileHandler>> createFileHandlerFuture = createFileHandlerPromise->get_future();
            //fileEngine->CreateFileHandlerAsync(inputFilePath, actualFilePath, false, sampleFileHandlerObserver, createFileHandlerPromise, fileExecutionState);
            fileEngine->CreateFileHandlerAsync(iStream1, actualFilePath, false, sampleFileHandlerObserver, createFileHandlerPromise, fileExecutionState);
            shared_ptr<FileHandler> fileHandler = createFileHandlerFuture.get();
            MessageBox(NULL, L"MIPInit", L"5", MB_OK);

            shared_ptr<ProtectionHandler> fileProtectionHandler = fileHandler->GetProtection();
            if (nullptr == fileProtectionHandler)
                MessageBox(NULL, L"GetProtection failed", L"6", MB_OK);
            else
                MessageBox(NULL, L"GetProtection success", L"6", MB_OK);

            //filebuf fb;
            fb.open("C:\\Daniel\\VC++\\MIPTest\\Test.txt", ios::in | ios::binary);
            //fb.open("C:\\Daniel\\VC++\\MIPTest\\Test1.txt", std::fstream::in | std::fstream::ate | std::fstream::binary);
            //shared_ptr<istream> ioStream = make_shared<istream>(&fb);
            //shared_ptr<Stream> iStream = CreateStreamFromStdStream(ioStream);
            ioStream = make_shared<istream>(&fb);
            iStream = CreateStreamFromStdStream(ioStream);
            if (nullptr != fileProtectionHandler)
                //protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x2ccf, 0x1180);
                protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x7891, 0x20);
            //shared_ptr<Stream> protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x5847, 0x10);
            //uint8_t unData[3];
            //protectedStream->Seek(1);
            //protectedStream->Read(unData, 3);
            MessageBox(NULL, L"MIPInit", L"6", MB_OK);
        }
        catch (const cxxopts::OptionException& ex) {
            cout << "Error parsing options: " << ex.what() << endl;
            OutputDebugStringA(ex.what());
            MessageBoxA(NULL, ex.what(), "7", MB_OK);
            return;
        }
        catch (const std::exception& ex) {
            cout << "Something bad happened: " << ex.what() << "\nExiting." << endl;
            OutputDebugStringA("**********************************");
            OutputDebugStringA(ex.what());
            MessageBoxA(NULL, ex.what(), "7", MB_OK);
            OutputDebugStringA("**********************************");
            return;
        }

    }

    BOOL MIPReadProtectedFile(LPVOID lpBuffer,
        DWORD        nNumberOfBytesToRead,
        LPDWORD      lpNumberOfBytesRead)
    {
        if (nullptr != protectedStream)
            *lpNumberOfBytesRead = protectedStream->Read((uint8_t*)lpBuffer, nNumberOfBytesToRead);
        return TRUE;
    }
}
