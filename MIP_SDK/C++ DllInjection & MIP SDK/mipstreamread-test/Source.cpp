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
        string sccToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyIsImtpZCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyJ9.eyJhdWQiOiJodHRwczovL3N5bmNzZXJ2aWNlLm8zNjVzeW5jc2VydmljZS5jb20vIiwiaXNzIjoiaHR0cHM6Ly9zdHMud2luZG93cy5uZXQvMGY4OTljZGQtZDI0NC00NTMyLTk0YjMtNjk4MmNmMTMwNjI3LyIsImlhdCI6MTY5MzcyMzA3MiwibmJmIjoxNjkzNzIzMDcyLCJleHAiOjE2OTM3MjcxNzIsImFjciI6IjEiLCJhaW8iOiJBVFFBeS84VUFBQUFRZTlRbis1SEI2RkhVVm9CZmE0OGp5Y1dxZmh1UGp3YTJpcVppcHAwdFJSMm54aW9pUmt2MDE3MmkydURKTmg5IiwiYW1yIjpbInB3ZCJdLCJhcHBpZCI6IjZiMDY5ZWVmLTlkZGUtNGEyOS1iNDAyLThjZTg2NmVkYzg5NyIsImFwcGlkYWNyIjoiMCIsImdpdmVuX25hbWUiOiJDYWRhdXRob3IiLCJpcGFkZHIiOiI0OS4yNDkuMTc0LjU0IiwibmFtZSI6IkNhZGF1dGhvciIsIm9pZCI6IjM1ZDY3ODBhLTM0MmUtNDU2Yi04ZWYzLTZhNmMyZjk0ODhjYiIsInB1aWQiOiIxMDAzMjAwMkFGN0ZEREM3IiwicmgiOiIwLkFVc0EzWnlKRDBUU01rV1VzMm1DenhNR0p5NVBESWUyaFVOTnZkcHUyYVY1dHlWTEFQQS4iLCJzY3AiOiJ1c2VyX2ltcGVyc29uYXRpb24iLCJzdWIiOiJzdmswMGhiWTF4eS01bHZpa2ZSdVdaeGVmUERUVnVnYjdERmtFR3NBRktJIiwidGlkIjoiMGY4OTljZGQtZDI0NC00NTMyLTk0YjMtNjk4MmNmMTMwNjI3IiwidW5pcXVlX25hbWUiOiJDYWRhdXRob3JAaGFsb3NlY3VkZS5vbm1pY3Jvc29mdC5jb20iLCJ1cG4iOiJDYWRhdXRob3JAaGFsb3NlY3VkZS5vbm1pY3Jvc29mdC5jb20iLCJ1dGkiOiI3YmlkaU5BekRVR0dDbWVNOVVoUkFnIiwidmVyIjoiMS4wIn0.RgmW7uZpjazEELhk8_bNW37p42Z3TIGvEe_tsiAfMFBn9ZO8_Z5L3PxT_m3dWF0bnzdVoWBSsSuWHSC4cRlDgWI4jx506gHSBGERfu0eK5Y6mnT5Kkuv7QFmbK0OcqXqz3-iF7MOQSZioUKzm600jWLrRxfmUjvXi5VGUbdaUbABdm3NbWKJYvR1jZGZAG9VFheTo_t_FFDEUeRslxq6kXGTcCHkVW9UFTTGUOWCIPRJdsTy__Gcdi2DNs7stXmThgIIv25F7Aay_77NG8SBjunCFJb6zwylaWaa1l3QsP4-6Jy6_QwZNfJqX59vZ5jYOvaTc0naroXLMH4dchJKZQ";
        string protectionToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyIsImtpZCI6Ii1LSTNROW5OUjdiUm9meG1lWm9YcWJIWkdldyJ9.eyJhdWQiOiJodHRwczovL2FhZHJtLmNvbSIsImlzcyI6Imh0dHBzOi8vc3RzLndpbmRvd3MubmV0LzBmODk5Y2RkLWQyNDQtNDUzMi05NGIzLTY5ODJjZjEzMDYyNy8iLCJpYXQiOjE2OTM3MjMxMDEsIm5iZiI6MTY5MzcyMzEwMSwiZXhwIjoxNjkzNzI4MjUwLCJhY3IiOiIxIiwiYWlvIjoiQVRRQXkvOFVBQUFBRTJXVTFzdmd0Wm1tL3lwc2M3a1hnMXFoNHV1Rk5qVEc2YnZjcWp3Q3UxK0FtZVF5U3IxQXlNdEg4RDdSRGNpdSIsImFtciI6WyJwd2QiXSwiYXBwaWQiOiI2YjA2OWVlZi05ZGRlLTRhMjktYjQwMi04Y2U4NjZlZGM4OTciLCJhcHBpZGFjciI6IjAiLCJnaXZlbl9uYW1lIjoiQ2FkYXV0aG9yIiwiaXBhZGRyIjoiNDkuMjQ5LjE3NC41NCIsIm5hbWUiOiJDYWRhdXRob3IiLCJvaWQiOiIzNWQ2NzgwYS0zNDJlLTQ1NmItOGVmMy02YTZjMmY5NDg4Y2IiLCJwdWlkIjoiMTAwMzIwMDJBRjdGRERDNyIsInJoIjoiMC5BVXNBM1p5SkQwVFNNa1dVczJtQ3p4TUdKeElBQUFBQUFBQUF3QUFBQUFBQUFBQkxBUEEuIiwic2NwIjoidXNlcl9pbXBlcnNvbmF0aW9uIiwic3ViIjoiYWhQUGMtckNOcWZ3ZWVUZVhWUF9DSzRDd2RINFRxUnNDc2lSQWhaNXFncyIsInRpZCI6IjBmODk5Y2RkLWQyNDQtNDUzMi05NGIzLTY5ODJjZjEzMDYyNyIsInVuaXF1ZV9uYW1lIjoiQ2FkYXV0aG9yQGhhbG9zZWN1ZGUub25taWNyb3NvZnQuY29tIiwidXBuIjoiQ2FkYXV0aG9yQGhhbG9zZWN1ZGUub25taWNyb3NvZnQuY29tIiwidXRpIjoiN2daS0lveXVoMGV0NVRKSzFZc0JBUSIsInZlciI6IjEuMCJ9.hU7Hn55qSDJ4aFVUZb6xn2tXJWca5W5PmNHrsDkPvPu3Zb50wsQ1PgPjZrz8Rq_y11vrlkX_hcMeN1S_gb9wCji6JH1VWRaQUWhRDgDJ3yUOg_sqOGvokYY35cpycl5PoDTtG71-pxrFIyggURQgBVOoVNGG6d4leI0_vxw5aG4yJaInkdTWZTVJwaXP0NlP0Nd6E_7U8FIY0PlBuSja4sSYPiWeYmeOY7b3Je41m2uVrk-xOOPzwJXPzim4GVqaup-B5DT0fqIjvsXpMfGBS9Ksfdt7dyIixhfktJl1BoPbiLYG0Uln1PfO7CYvAtRHSXWbXWsuNN0ueAEmR2rI-w";
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