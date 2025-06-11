#define NOMINMAX

#include <Windows.h>
#include <winerror.h>

#include <stdio.h>

#include <iostream>
#include <fstream>
using namespace std;


#include <mip/mip_configuration.h>
#include <mip/mip_context.h>
#include <mip/file/file_profile.h>
#include <mip/file/file_engine.h>
#include <mip/stream_utils.h>
using namespace mip;

#include "consent/consent_delegate_impl.h"
using namespace sample::consent;

#include "file/profile_observer.h"
#include "file/file_handler_observer.h"
#include "file/file_execution_state_impl.h"
#include "common/auth_delegate_impl.h"
#include "common/cxxopts.hpp"
using namespace sample::auth;

#pragma comment(lib, "mip_file_sdk.lib")
#pragma comment(lib, "mip_protection_sdk.lib")
#pragma comment(lib, "mip_upe_sdk.lib")

void wmain()
{
    try
    {
        ApplicationInfo appInfo;
        appInfo.applicationId = "9ed1f576-dada-42f4-aa49-877b2e71bedd";
        appInfo.applicationName = "HALOCADApp";
        appInfo.applicationVersion = "1.0.0.0";
        shared_ptr<MipConfiguration> mipConfiguration = make_shared<MipConfiguration>(appInfo, ".", LogLevel::Error, false);
        // Create Context
        shared_ptr<MipContext> mipContext = MipContext::Create(mipConfiguration);

        // Create Profile
        shared_ptr<ConsentDelegate> consentDelegate = make_shared<ConsentDelegateImpl>(false);
        const shared_ptr<ProfileObserver> sampleProfileObserver = make_shared<ProfileObserver>();
        FileProfile::Settings fileProfileSettings(mipContext, CacheStorageType::OnDisk, consentDelegate, sampleProfileObserver);
        shared_ptr<std::promise<shared_ptr<FileProfile>>> loadPromise = make_shared<std::promise<shared_ptr<FileProfile>>>();
        future<shared_ptr<FileProfile>> loadFuture = loadPromise->get_future();
        FileProfile::LoadAsync(fileProfileSettings, loadPromise); // Getting the profile
        shared_ptr<FileProfile> fileProfile = loadFuture.get();

        // Create fileengine
        string sccToken = "";
        string protectionToken = "";
        shared_ptr<AuthDelegateImpl> authDelegate = make_shared<AuthDelegateImpl>(false /*isVerbose*/, "", "", "", sccToken, protectionToken, ".");
        FileEngine::Settings fileeEngineSettings(Identity("Cadauthor@halosecude.onmicrosoft.com"), authDelegate, "");
        fileeEngineSettings.SetCloud(mip::Cloud::Commercial);
        auto addEnginePromise = make_shared<std::promise<shared_ptr<FileEngine>>>();
        auto addEngineFuture = addEnginePromise->get_future();
        fileProfile->AddEngineAsync(fileeEngineSettings, addEnginePromise); // Getting the engine
        shared_ptr<FileEngine> fileEngine = addEngineFuture.get();

        // Create File handler
        string inputFilePath = "C:\\Daniel\\VC++\\MIPTest\\Test21.txt";
        string actualFilePath = "C:\\Daniel\\VC++\\MIPTest\\Test2.txt";
        //string inputFilePath = "D:\\Test\\Test\\CPPMIPSDKTestFiles\\Protected\\food.ptxt";
        //string actualFilePath = "D:\\Test\\Test\\CPPMIPSDKTestFiles\\Protected\\food.ptxt";
        const shared_ptr<FileHandlerObserver> sampleFileHandlerObserver = make_shared<FileHandlerObserver>();
        shared_ptr<FileExecutionStateImpl> fileExecutionState = make_shared<FileExecutionStateImpl>(DataState::REST, nullptr, false, "");
        shared_ptr<promise<shared_ptr<FileHandler>>> createFileHandlerPromise = make_shared<std::promise<shared_ptr<FileHandler>>>();
        future<shared_ptr<FileHandler>> createFileHandlerFuture = createFileHandlerPromise->get_future();
        fileEngine->CreateFileHandlerAsync(
            inputFilePath, actualFilePath, false, sampleFileHandlerObserver, createFileHandlerPromise, fileExecutionState);
        shared_ptr<FileHandler> fileHandler = createFileHandlerFuture.get();

        shared_ptr<ProtectionHandler> fileProtectionHandler = fileHandler->GetProtection();
        filebuf fb;
        fb.open("C:\\Daniel\\VC++\\MIPTest\\Test2.txt", ios::in | ios::out | ios::binary);
        //fb.open("D:\\Test\\Test\\CPPMIPSDKTestFiles\\Protected\\food.ptxt", ios::in | ios::binary);
        shared_ptr<istream> ioStream = make_shared<istream>(&fb);
        shared_ptr<Stream> iStream = CreateStreamFromStdStream(ioStream);
        //shared_ptr<Stream> protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x2ccf, 0x1180);
        //shared_ptr<Stream> protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x5847, 0x10);
        //shared_ptr<Stream> protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x7891, 0x20); // Test1.txt
        shared_ptr<Stream> protectedStream = fileProtectionHandler->CreateProtectedStream(iStream, 0x7891, 0x1f0010); // Test2.txt
        uint8_t unData[33];
        SecureZeroMemory(unData, 33);
        protectedStream->Seek(0);
        cout << "Position:" << protectedStream->Position() << endl;
        cout << "Size:" << protectedStream->Size() << endl;
        // Find the actual buffer size
        for (int i = 0; i < 16; i++)
        {
            protectedStream->Seek(protectedStream->Size() - i);
            //cout << "Position:" << protectedStream->Position() << endl;
            //cout << "Data read:" << protectedStream->Read(unData, 1) << endl;
            if (0 != protectedStream->Read(unData, 1))
                break;
        }
        //cout << "Data read:" << protectedStream->Read(unData, 32) << endl;
        cout << "Data:" << unData << endl;
        cout << "Size:" << protectedStream->Size() << endl;
        cout << "Position:" << protectedStream->Position() << endl;
    } catch (const cxxopts::OptionException& ex) {
        cout << "Error parsing options: " << ex.what() << endl;
        return;
    }
    catch (const std::exception& ex) {
        cout << "Something bad happened: " << ex.what() << "\nExiting." << endl;
        return;
    }

}
