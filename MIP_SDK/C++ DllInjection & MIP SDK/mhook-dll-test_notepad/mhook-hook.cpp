
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
            string sccToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6IkNOdjBPSTNSd3FsSEZFVm5hb01Bc2hDSDJYRSIsImtpZCI6IkNOdjBPSTNSd3FsSEZFVm5hb01Bc2hDSDJYRSJ9.eyJhdWQiOiJodHRwczovL3N5bmNzZXJ2aWNlLm8zNjVzeW5jc2VydmljZS5jb20iLCJpc3MiOiJodHRwczovL3N0cy53aW5kb3dzLm5ldC8wZjg5OWNkZC1kMjQ0LTQ1MzItOTRiMy02OTgyY2YxMzA2MjcvIiwiaWF0IjoxNzQ4NzY4NjcwLCJuYmYiOjE3NDg3Njg2NzAsImV4cCI6MTc0ODc3MzUwNCwiYWNyIjoiMSIsImFpbyI6IkFVUUF1LzhaQUFBQVZ3OGhleEd5REp0VzZLbEk1L3BzemtCSzg2UFVKYUExaXRMLzlHaXcrdzAxN2hzbzNlbW5VbkNIZmlwSHpRVldRVGVMOXUvVENZWlBBcUxEdnA4SzRBPT0iLCJhbXIiOlsicHdkIl0sImFwcGlkIjoiOWVkMWY1NzYtZGFkYS00MmY0LWFhNDktODc3YjJlNzFiZWRkIiwiYXBwaWRhY3IiOiIwIiwiZ2l2ZW5fbmFtZSI6IkNhZGF1dGhvciIsImlkdHlwIjoidXNlciIsImlwYWRkciI6IjQ5LjI0OS4xNzQuNTQiLCJuYW1lIjoiQ2FkYXV0aG9yIiwib2lkIjoiMzVkNjc4MGEtMzQyZS00NTZiLThlZjMtNmE2YzJmOTQ4OGNiIiwicHVpZCI6IjEwMDMyMDAyQUY3RkREQzciLCJyaCI6IjEuQVVzQTNaeUpEMFRTTWtXVXMybUN6eE1HSnk1UERJZTJoVU5OdmRwdTJhVjV0eVc4QWZCTEFBLiIsInNjcCI6IlVuaWZpZWRQb2xpY3kuVXNlci5SZWFkIiwic2lkIjoiMDA1YmJjNzktMzZiNi04MTAyLWFiYmMtYzYzMjcxYjU0ZTU4Iiwic3ViIjoic3ZrMDBoYlkxeHktNWx2aWtmUnVXWnhlZlBEVFZ1Z2I3REZrRUdzQUZLSSIsInRpZCI6IjBmODk5Y2RkLWQyNDQtNDUzMi05NGIzLTY5ODJjZjEzMDYyNyIsInVuaXF1ZV9uYW1lIjoiQ2FkYXV0aG9yQGhhbG9zZWN1ZGUub25taWNyb3NvZnQuY29tIiwidXBuIjoiQ2FkYXV0aG9yQGhhbG9zZWN1ZGUub25taWNyb3NvZnQuY29tIiwidXRpIjoiOV8yUkVyZzYzMHVUVjBZcDBOTkxBQSIsInZlciI6IjEuMCIsInhtc19mdGQiOiI1d3I4QWpYS0hjVWN4di1lUDQ1RFBVWVNiYkkyTmZBQWhtWjUxME9jckdRQlpYVnliM0JsYm05eWRHZ3RaSE50Y3ciLCJ4bXNfaWRyZWwiOiIxIDI4In0.cKFSqCvSHhbcg5J4x25UWCnhOdxE-bWeR8LnCe4Yz3j-N9XxiTW4ru_dI6rfNCiNF_a4_49XsmIG2WOszExblJ5cpirFEVv5hytUoKWDoxoYG6CVRQ7vB05tUfx_lbZ-La-EPxknkam-kfzvaycMTCYzt51zxHdYsULro7Z9EXuz7GSyKxkbl55zFhvHoiSF9CBjaOg1f-4NjE34xCON4jyW3CLSZ-LUI0xyeeIL37gwZzA-kxhFr1GKk_JF6Y8n1XesAPT3BxmpLbS6-N2uUaljvbo9bH3wAADEFdO6TEnLsrMEb77Y0GTEbUJYuVeOOxxr3Ff5CIuMmfaA8AIfWw";
            string protectionToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6IkNOdjBPSTNSd3FsSEZFVm5hb01Bc2hDSDJYRSIsImtpZCI6IkNOdjBPSTNSd3FsSEZFVm5hb01Bc2hDSDJYRSJ9.eyJhdWQiOiJodHRwczovL2FwaS5hYWRybS5jb20iLCJpc3MiOiJodHRwczovL3N0cy53aW5kb3dzLm5ldC8wZjg5OWNkZC1kMjQ0LTQ1MzItOTRiMy02OTgyY2YxMzA2MjcvIiwiaWF0IjoxNzQ4NzY4Njc4LCJuYmYiOjE3NDg3Njg2NzgsImV4cCI6MTc0ODc3NDIwNSwiYWNyIjoiMSIsImFpbyI6IkFVUUF1LzhaQUFBQVFDQlFGc1p3KzlFem9yZWpUdU5UMVBGSnBMQkh6OFV4TTFjV3U4V0Z5UVZEVm5pY1YzVFk4R0JsYTZrd1pTeDlwOFY1eEVuRVEvSWdpQi93QW5zd3dRPT0iLCJhbXIiOlsicHdkIl0sImFwcGlkIjoiOWVkMWY1NzYtZGFkYS00MmY0LWFhNDktODc3YjJlNzFiZWRkIiwiYXBwaWRhY3IiOiIwIiwiZ2l2ZW5fbmFtZSI6IkNhZGF1dGhvciIsImlkdHlwIjoidXNlciIsImlwYWRkciI6IjQ5LjI0OS4xNzQuNTQiLCJuYW1lIjoiQ2FkYXV0aG9yIiwib2lkIjoiMzVkNjc4MGEtMzQyZS00NTZiLThlZjMtNmE2YzJmOTQ4OGNiIiwicHVpZCI6IjEwMDMyMDAyQUY3RkREQzciLCJyaCI6IjEuQVVzQTNaeUpEMFRTTWtXVXMybUN6eE1HSnhJQUFBQUFBQUFBd0FBQUFBQUFBQUM4QWZCTEFBLiIsInNjcCI6InVzZXJfaW1wZXJzb25hdGlvbiIsInNpZCI6IjAwNWJiYzc5LTM2YjYtODEwMi1hYmJjLWM2MzI3MWI1NGU1OCIsInN1YiI6ImFoUFBjLXJDTnFmd2VlVGVYVlBfQ0s0Q3dkSDRUcVJzQ3NpUkFoWjVxZ3MiLCJ0aWQiOiIwZjg5OWNkZC1kMjQ0LTQ1MzItOTRiMy02OTgyY2YxMzA2MjciLCJ1bmlxdWVfbmFtZSI6IkNhZGF1dGhvckBoYWxvc2VjdWRlLm9ubWljcm9zb2Z0LmNvbSIsInVwbiI6IkNhZGF1dGhvckBoYWxvc2VjdWRlLm9ubWljcm9zb2Z0LmNvbSIsInV0aSI6IkwzSnNtZjZCLVVhSDFOZGdJeFJEQUEiLCJ2ZXIiOiIxLjAiLCJ4bXNfZnRkIjoibHp2WUh2OTVBeDhfNV91YmsxTWpWQUhZVDI1a2F1dmdJMk5ROWtEc05TZ0JabkpoYm1ObFl5MWtjMjF6IiwieG1zX2lkcmVsIjoiMSAyNiJ9.TDDw_TzDVkNJ4SxebvLSlYRzXe4eq6Og1uw99H4TbKOk307K1Q56zsqeCAsHlvfYj8VI3TTJSoeas8Zn9sV_R1155gpQwOw0UiSZcFZG4jt0d1OvlvS1Agpi2NZ170C_LSTOCMha6A7f1f8BJNOOYcPvMbAycZ1yIFv7lo-dg0Erky6fGAeak_YBdCTEsOiJvjj4Yr_eG5YySHjTtznWzqiyO46XoZ83xVtTJlaY0DGmjZLyuGo6_MaaZvtPK5uJ7psyu07gNuJBrUBxBrajkWUQe-uFQ6AwbWVZyLWayM5hIB9MSSvoCYpg2g_xzaVwsYTHWok2fU0jZj6j_ZQPWw";
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