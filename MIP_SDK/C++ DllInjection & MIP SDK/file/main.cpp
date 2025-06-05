/**
 *
 * Copyright (c) Microsoft Corporation.
 * All rights reserved.
 *
 * This code is licensed under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <codecvt>

#ifdef __linux__
#include <unistd.h>
#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
#endif // __linux__

#pragma comment(lib, "mip_file_sdk.lib")
#pragma comment(lib, "mip_protection_sdk.lib")
#pragma comment(lib, "mip_upe_sdk.lib")

#include "cxxopts.hpp"

#include "auth_delegate_impl.h"
#include "consent_delegate_impl.h"
#include "file_execution_state_impl.h"
#include "file_handler_observer.h"
#include "stream_over_buffer.h"
#include "mip/common_types.h"
#include "mip/error.h"
#include "mip/file/file_handler.h"
#include "mip/file/file_inspector.h"
#include "mip/file/file_profile.h"
#include "mip/file/file_status.h"
#include "mip/file/labeling_options.h"
#include "mip/file/msg_inspector.h"
#include "mip/mip_context.h"
#include "mip/protection_descriptor.h"
#include "mip/protection/protection_descriptor_builder.h"
#include "mip/protection/protection_handler.h"
#include "mip/protection/rights.h"
#include "mip/stream_utils.h"
#include "mip/stream_utils.h"
#include "mip/upe/policy_engine.h"
#include "mip/user_rights.h"
#include "mip/user_roles.h"
#include "mip/version.h"
#include "profile_observer.h"
#include "shutdown_manager.h"
#include "string_utils.h"

using mip::ActionSource;
using mip::ApplicationInfo;
using mip::AssignmentMethod;
using mip::AuthDelegate;
using mip::CacheStorageType;
using mip::ConsentDelegate;
using mip::DataState;
using mip::FileEngine;
using mip::FileHandler;
using mip::FileInspector;
using mip::FileProfile;
using mip::FileStatus;
using mip::Identity;
using mip::InspectorType;
using mip::Label;
using mip::LabelingOptions;
using mip::MipContext;
using mip::MsgInspector;
using mip::NoPermissionsError;
using mip::PolicyEngine;
using mip::ProtectionDescriptor;
using mip::ProtectionDescriptorBuilder;
using mip::ProtectionHandler;
using mip::Stream;
using mip::UserRights;
using mip::UserRoles;
using sample::auth::AuthDelegateImpl;
using sample::consent::ConsentDelegateImpl;
using std::cin;
using std::codecvt_utf8_utf16;
using std::cout;
using std::endl;
using std::fstream;
using std::get_time;
using std::getline;
using std::ifstream;
using std::iostream;
using std::istream;
using std::make_shared;
using std::map;
using std::ostream_iterator;
using std::ostringstream;
using std::pair;
using std::put_time;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::stringstream;
using std::tm;
using std::vector;
using std::wstring_convert;
using std::wstring;

namespace {

static const string kApplicationName = "Microsoft Information Protection File SDK Sample";

static const char kPathSeparatorWindows = '\\';
static const char kPathSeparatorUnix = '/';
static const char kExtensionSeparator = '.';

// Explicit null character at the end is required since array initializer does NOT add it.

static const char kPathSeparatorCStringWindows[] = {kPathSeparatorWindows, '\0'};
static const char kPathSeparatorCStringUnix[] = {kPathSeparatorUnix, '\0'};
static const char kPathSeparatorsAll[] = {kPathSeparatorWindows, kPathSeparatorUnix, '\0'};

const char* whiteSpaceCharacters = " \t\r\n";
void Trim(string& a, const char* trim_char = whiteSpaceCharacters) {
  auto pos = a.find_last_not_of(trim_char);
  if (pos != string::npos) {
    a.erase(pos + 1);
    pos = a.find_first_not_of(trim_char);
    if (pos != string::npos) a.erase(0, pos);
  } else
    a.erase(a.begin(), a.end());
}

vector<string> SplitString(const string& str, char delim) {
  vector<string> output;
  stringstream ss(str);
  string substr;

  while (ss.good()) {
    getline(ss, substr, delim);
    output.emplace_back(move(substr));
  }

  return output;
}

map<string, string> SplitDict(const string& str) {
  map<string, string> dict;
  auto entries = SplitString(str, ',');
  for (const auto& entry : entries) {
    auto kv = SplitString(entry, ':');
    if (kv.size() == 2)
      dict[kv[0]] = kv[1];
  }
  return dict;
}

string GetFileName(const string& filePath) {
  auto index = filePath.find_last_of(kPathSeparatorsAll);
  if (index == string::npos) return filePath;
  return filePath.substr(index + 1);
}

string GetFileExtension(const string& filePath) {
  string fileName = GetFileName(filePath);
  auto index = fileName.rfind(kExtensionSeparator);
  if (index == string::npos) return "";
  return fileName.substr(index); // Include the dot in the file extension
}

shared_ptr<FileStatus> GetFileStatus(const string& filePath, const shared_ptr<Stream>& fileStream, const shared_ptr<MipContext>& mipContext) {
  if (fileStream) {
    return FileHandler::GetFileStatus(fileStream, filePath, mipContext);
  } else {
    return FileHandler::GetFileStatus(filePath, mipContext);
  }
}

bool EqualsIgnoreCase(const string& a, const string& b) {
  auto size = a.size();
  if (b.size() != size) {
    return false;
  }
    
  for (size_t i = 0; i < size; i++) {
    if (tolower(a[i]) != tolower(b[i])) {
      return false;
    }
  }

  return true;
}

map<mip::FlightingFeature, bool> SplitFeatures(const string& str) {
  map<mip::FlightingFeature, bool> dict;
  auto entries = SplitString(str, ',');
  for (const auto& entry : entries) {
    auto kv = SplitString(entry, ':');
    if (kv.size() == 2)
      dict[(mip::FlightingFeature)stoul(kv[0])] = (EqualsIgnoreCase(kv[1], "true") || (kv[1] == "1")) ? true : false;
    else
      throw std::runtime_error(string("FlightingFeature has wrong format: ") + entry);
  }
  return dict;
}

//saving a file memory in file buffer
shared_ptr<mip::Stream> GetInputStreamFromFilePath(const string& filePath) {
#ifdef _WIN32
  auto ifStream = make_shared<fstream>(ConvertStringToWString(filePath), std::ios_base::in | std::ios_base::binary);
#else // _WIN32
  auto ifStream = make_shared<fstream>(filePath, std::ios_base::in | std::ios_base::binary);
#endif
  uint8_t buffer[4096] = {0};
  std::streamsize bytesRead = 0;
  vector<uint8_t> memoryFile;
  do {
    ifStream->clear();
    ifStream->read(reinterpret_cast<char*>(buffer), sizeof(buffer));
    bytesRead = ifStream->gcount();
    if (bytesRead > 0) {
      memoryFile.insert(memoryFile.end(), &buffer[0], &buffer[bytesRead]);
    }
  } while (bytesRead > 0);
  return make_shared<StreamOverBuffer>(move(memoryFile));
}
// Get the current label and protection on this file and print to console label and protection information

void GetLabel(
  const shared_ptr<FileHandler>& fileHandler) {
  auto protection = fileHandler->GetProtection(); // Get the current protection on the file
  auto label = fileHandler->GetLabel(); //Get the current label on the file

  if (!label && !protection) {
    cout << "File is neither labeled nor protected" << endl;
    return;
  }

  if (label) {
    bool isPrivileged = label->GetAssignmentMethod() == AssignmentMethod::PRIVILEGED;
    auto extendedProperties = label->GetExtendedProperties();
    cout << "File is labeled as: " << label->GetLabel()->GetName() << endl;
    cout << "Id: " << label->GetLabel()->GetId() << endl;

    if (const shared_ptr<mip::Label> parent = label->GetLabel()->GetParent().lock()) {
      cout << "Parent label: " << parent->GetName() << endl;
      cout << "Parent Id: " << parent->GetId() << endl;
    }
    auto time = std::chrono::system_clock::to_time_t(label->GetCreationTime());
    std::stringstream ss;
    ss << time;
    cout << "Set time: " << ss.str() << endl;
    cout << "Privileged: " << (isPrivileged ? "True" : "False") << endl;
    if (!extendedProperties.empty()) {
      cout << "Extended Properties: " << endl;
    }
    for (size_t j = 0; j < extendedProperties.size(); j++) {
      cout << "Key: " << extendedProperties[j].GetKey() <<
          ", Value: " << extendedProperties[j].GetValue() << endl;
    }

  } else
    cout << "File is not labeled by an official policy" << endl;

  if (protection) {
    cout << "File is protected with ";

    const shared_ptr<ProtectionDescriptor> protectionDescriptor = protection->GetProtectionDescriptor();
    if (protectionDescriptor->GetProtectionType() == mip::ProtectionType::TemplateBased)
      cout << "template." << endl;
    else
      cout << "custom permissions." << endl;

    cout << "Name: " << protectionDescriptor->GetName() << endl;
    cout << "Template Id: " << protectionDescriptor->GetTemplateId() << endl;

    for (const auto& usersRights : protectionDescriptor->GetUserRights()) {
      cout << "Rights: ";
      auto rights = usersRights.Rights();
      copy(rights.cbegin(), rights.cend() - 1, ostream_iterator<string>(cout, ", "));
      cout << *rights.crbegin() << endl;

      cout << "For Users: ";
      auto users = usersRights.Users();
      copy(users.cbegin(), users.cend() - 1, ostream_iterator<string>(cout, "; "));
      cout << *users.crbegin() << endl;
    }

    for (const auto& usersRoles : protectionDescriptor->GetUserRoles()) {
      cout << "Roles: ";
      auto roles = usersRoles.Roles();
      copy(roles.cbegin(), roles.cend() - 1, ostream_iterator<string>(cout, ", "));
      cout << *roles.crbegin() << endl;

      cout << "For Users: ";
      auto users = usersRoles.Users();
      copy(users.cbegin(), users.cend() - 1, ostream_iterator<string>(cout, "; "));
      cout << *users.crbegin() << endl;
    }

    if (protectionDescriptor->DoesContentExpire()) {
      time_t validUntilTime = std::chrono::system_clock::to_time_t(protectionDescriptor->GetContentValidUntil());
      tm validUntilUtc = {};
      tm validUntil = {};
#ifdef _WIN32
      gmtime_s(&validUntilUtc, &validUntilTime);
      localtime_s(&validUntil, &validUntilTime);
#else
      gmtime_r(&validUntilTime, &validUntilUtc);
      localtime_r(&validUntilTime, &validUntil);
#endif
      cout << "Content Expiration (UTC): " << put_time(&validUntilUtc, "%FT%TZ") << endl;
      cout << "Content Expiration: " << put_time(&validUntil, "%FT%T%z") << endl;
    }
  }
}

string CreateOutput(FileHandler* fileHandler) {
  auto outputFileName = fileHandler->GetOutputFileName();
  auto fileExtension = GetFileExtension(outputFileName);
  auto outputFileNameWithoutExtension = outputFileName.substr(0, outputFileName.length() - fileExtension.length());

  //if (EqualsIgnoreCase(fileExtension, ".pfile")) {        //Modified to remove pfile extension
  if (EqualsIgnoreCase(fileExtension,"")) {
    fileExtension = GetFileExtension(outputFileNameWithoutExtension) + fileExtension;
    outputFileNameWithoutExtension = outputFileName.substr(0, outputFileName.length() - fileExtension.length());
  }

  return outputFileNameWithoutExtension + "_modified" + fileExtension;
}

char PathSeparator() {
#ifdef _WIN32
  return kPathSeparatorWindows;
#else
  return kPathSeparatorUnix;
#endif
}

string GetDirFromPath(const string& fileSamplePath) {
  string result;
  if (!fileSamplePath.empty()) {
    auto position = fileSamplePath.find_last_of(PathSeparator());
    result = fileSamplePath.substr(0, position + 1);
  }
  return result;
}

string CombinePaths(const string& folder, const string& relativePath) {
  if (folder.empty()) return relativePath;
  string result = folder;
  if (result[result.length() - 1] != PathSeparator()) result += PathSeparator();
  return result + relativePath;
}

void WriteAttachment(const std::shared_ptr<mip::MsgAttachmentData>& attachment, const string& directory) {
  static const size_t kStreamBufferSize = 4096;
  if (!attachment)
    return;
  auto stream = attachment->GetStream();
  if (!stream)
    return;
  stream->Seek(0);
  auto filePath = CombinePaths(directory, attachment->GetLongName());
  #ifdef _WIN32
    shared_ptr<iostream> ioStream(new fstream(ConvertStringToWString(filePath), std::ios_base::out | std::ios_base::binary));
  #else // _WIN32
    shared_ptr<iostream> ioStream(new fstream(filePath, std::ios_base::out | std::ios_base::binary));
  #endif

  auto outputStream = mip::CreateStreamFromStdStream(ioStream);
  vector<uint8_t> buffer(kStreamBufferSize);
  int64_t currentRead = 0;
  int64_t currentTempWrite = 0;
  int64_t currentWrite = 0;
  do {
    currentRead = stream->Read(buffer.data(), kStreamBufferSize);
    currentWrite = 0;
    if (currentRead > 0) {
      do {
        currentTempWrite = outputStream->Write(buffer.data() + currentWrite, currentRead - currentWrite);
        currentWrite += currentTempWrite;
      } while (currentTempWrite > 0 && currentRead != currentWrite);
    }
  } while (currentRead > 0);
}

void SetLabel(
  const shared_ptr<FileHandler>& fileHandler,
  const shared_ptr<Label>& label,
  const string& filePath,
  const std::shared_ptr<Stream>& outfileStream,
  AssignmentMethod method,
  const string& justificationMessage,
  const vector<pair<string, string>>& extendedProperties) {

  LabelingOptions labelingOptions(method);
  labelingOptions.SetDowngradeJustification(!justificationMessage.empty(), justificationMessage);
  labelingOptions.SetExtendedProperties(extendedProperties);

  if (label == nullptr) {
    fileHandler->DeleteLabel(labelingOptions); // Delete the current label from the file
  } else {
    fileHandler->SetLabel(label, labelingOptions, mip::ProtectionSettings());  // Set a label with label Id to the file
  }

  auto outputFilePath = CreateOutput(fileHandler.get());

  auto commitPromise = make_shared<std::promise<bool>>();
  auto commitFuture = commitPromise->get_future();
  auto modified = fileHandler->IsModified();
 
  if (modified) {
      if (nullptr == outfileStream)
        fileHandler->CommitAsync(outputFilePath, commitPromise);
      else
        fileHandler->CommitAsync(outfileStream, commitPromise);
    auto committed = commitFuture.get();

    if (committed) {
      cout << "New file created: " << outputFilePath << endl;
      //Triggers audit event
      fileHandler->NotifyCommitSuccessful(filePath);
    } else {
       if (remove(outputFilePath.c_str()) != 0) {
         throw std::runtime_error("unable to delete outputfile");
       }
      }
  } else {
      cout << "No changes to commit" << endl;
	}

}

void Inspect(
  const shared_ptr<MipContext>& mipContext,
  shared_ptr<Stream> fileStream,
  const shared_ptr<FileHandler>& fileHandler,
  const string& filePath,
  bool writeAttachments) {
  cout << filePath << endl;
  auto fileStatus = GetFileStatus(filePath, fileStream, mipContext);
  auto isProtected = fileStatus->IsProtected();
  // Note that only checking if the file is protected does not require any network IO or auth
  if (!isProtected) {
    cout << "File is not protected, no change made." << endl;
    return;
  }
  auto commitPromise = make_shared<std::promise<shared_ptr<FileInspector>>>();
  auto commitFuture = commitPromise->get_future();
  fileHandler->InspectAsync(commitPromise);
  auto inspector = commitFuture.get();
  if (inspector) {
    MsgInspector* msgInspector = nullptr;
    cout << "New inspector created: " << endl;
    switch (inspector->GetInspectorType()) {
      case InspectorType::Msg:
        cout << "Created Msg Inspector" << endl;
        msgInspector = static_cast<MsgInspector*>(inspector.get());
        cout << "Message body size:" << msgInspector->GetBody().size() << endl;
        cout << "Message body body code page :" << msgInspector->GetCodePage() << endl;
        cout << "Message attachments count :" << msgInspector->GetAttachments().size() << endl;
        for (const auto& attachment : msgInspector->GetAttachments()) {
          cout << "Attachment Name:" << attachment->GetName() << endl;
          cout << "Attachment Long Name:" << attachment->GetLongName() << endl;
          cout << "Attachment Path:" << attachment->GetPath() << endl;
          cout << "Attachment Long Path:" << attachment->GetLongPath() << endl;
          cout << "Attachment Size:" << attachment->GetBytes().size() << endl;
          if (writeAttachments)
            WriteAttachment(attachment, GetDirFromPath(filePath));
        }
        break;
      case InspectorType::Unknown:
        throw std::runtime_error("Unable to create inspector to this file.");
        break;
      default:
        throw std::runtime_error("Unable to create inspector to this file.");
        break;
    } 
  } else {
    throw std::runtime_error("Unable to create inspector to this file.");  
  }
}

void Unprotect(
  const shared_ptr<MipContext>& mipContext,
  const shared_ptr<FileHandler>& fileHandler,
  shared_ptr<Stream> fileStream,
  const string& filePath) {
  cout << filePath << endl;
  auto fileStatus = GetFileStatus(filePath, fileStream, mipContext);
  auto isProtected = fileStatus->IsProtected();
  auto containsProtectedObjects = fileStatus->ContainsProtectedObjects();
  // Note that only checking if the file is protected does not require any network IO or auth
  if (!isProtected && !containsProtectedObjects) {
    cout << "File is not protected and does not contain protected objects, no change made." << endl;
    return;
  }

  fileHandler->RemoveProtection(); // Remove the protection from the file
  auto outputFilePath = CreateOutput(fileHandler.get());

  auto commitPromise = make_shared<std::promise<bool>>();
  auto commitFuture = commitPromise->get_future();
  auto modified = fileHandler->IsModified();
  if (modified) {
    fileHandler->CommitAsync(outputFilePath, commitPromise);
    auto committed = commitFuture.get();

    if (committed) {
      cout << "New file created: " << outputFilePath << endl;
    } else {
      if (remove(outputFilePath.c_str()) != 0) {
        throw std::runtime_error("unable to delete outputfile");
      }
    }
  } else {
    cout << "No changes to commit" << endl;
  }
}

// Print the labels and sublabels to the console
void ListLabels(const vector<shared_ptr<mip::Label>>& labels, const string& delimiter = "") {
  static const size_t kMaxTooltipSize = 70;
  for (const auto& label : labels) {
    string labelTooltip = label->GetTooltip();
    if (labelTooltip.size() > kMaxTooltipSize)
      labelTooltip.substr(0, kMaxTooltipSize) + "...";
    
    string labelAutoTooltip = label->GetAutoTooltip();
    if (labelAutoTooltip.size() > kMaxTooltipSize)
      labelAutoTooltip.substr(0, kMaxTooltipSize) + "...";
    cout << delimiter << "Label ID: " << label->GetId() << "\n" <<
        delimiter << "Label name: " << label->GetName() << "\n" <<
        delimiter << "Label sensitivity: " << label->GetSensitivity() << "\n" <<
        delimiter << "Label tooltip: " << labelTooltip << "\n" <<
        delimiter << "Label autoTooltip: " << labelAutoTooltip << "\n" << endl;
    
    const vector<shared_ptr<mip::Label>>& childLabels = label->GetChildren();
    if (!childLabels.empty()) {
      cout << delimiter << "Child labels:" << endl;
      ListLabels(childLabels, delimiter + "  ");
    }
  }
}

void ProtectWithPermissions(
  const shared_ptr<FileHandler>& fileHandler,
  const shared_ptr<ProtectionDescriptorBuilder> descriptorBuilder) {
  const auto protectionDescriptor = descriptorBuilder->Build();
  fileHandler->SetProtection(protectionDescriptor, mip::ProtectionSettings());
  auto outputFilePath = CreateOutput(fileHandler.get());

  auto commitPromise = make_shared<std::promise<bool>>();
  auto commitFuture = commitPromise->get_future();
  fileHandler->CommitAsync(outputFilePath, commitPromise);
  auto committed = commitFuture.get();

  if (committed) {
    cout << "New file created: " << outputFilePath << endl;
  } else {
    if (remove(outputFilePath.c_str()) != 0) {
      throw std::runtime_error("unable to delete outputfile");
    }
  }
}

void ProtectWithCustomPermissions(
  const shared_ptr<FileHandler>& fileHandler,
  const string& usersList, 
  const string& rightsList,
  const string& rolesList,
  const string& expiration) {
  vector<string> userList;
  stringstream usersListstream(usersList);
  while (usersListstream.good())
  {
    string substr;
    getline(usersListstream, substr, ',');
    userList.push_back(substr);
  }

  vector<string> rightList;
  stringstream rightsListstream(rightsList);
  while (rightsListstream.good())
  {
    string right;
    getline(rightsListstream, right, ',');
    if (!right.empty())
      rightList.push_back(right);
  }

  vector<string> roleList;
  stringstream rolesListstream(rolesList);
  while (rolesListstream.good())
  {
    string role;
    getline(rolesListstream, role, ',');
    if (!role.empty())
      roleList.push_back(role);
  }

  shared_ptr<ProtectionDescriptorBuilder> protectionDescriptorBuilder;
  if (rightList.empty()) {
    const UserRoles userRoles(userList, roleList);
    protectionDescriptorBuilder = ProtectionDescriptorBuilder::CreateFromUserRoles(vector<UserRoles>({ userRoles }));
  } else {
    const UserRights usersRights(userList, rightList);
    protectionDescriptorBuilder = ProtectionDescriptorBuilder::CreateFromUserRights(vector<UserRights>({ usersRights }));
  }

  if (!expiration.empty()) {
    tm validUntilTm = {};
    stringstream stream(expiration);
    stream >> get_time(&validUntilTm, "%Y-%m-%dT%H:%M:%S");
#ifdef _WIN32
    time_t validUtilTime = _mkgmtime(&validUntilTm);
#else
    time_t validUtilTime = timegm(&validUntilTm);
#endif
    protectionDescriptorBuilder->SetContentValidUntil(std::chrono::system_clock::from_time_t(validUtilTime));
  }

  ProtectWithPermissions(fileHandler, protectionDescriptorBuilder);
 }

string ReadPolicyFile(const string& policyPath) {
  ifstream ifs(FILENAME_STRING(policyPath));
  if (ifs.fail())
    throw std::runtime_error("Failed to read path: " + policyPath);

  cout << "Using policy from file: " << policyPath << endl;

  ostringstream policyContent;
  policyContent << ifs.rdbuf();
  ifs.close();
  return policyContent.str();
}

void EnsureUserHasRights(const shared_ptr<FileHandler>& fileHandler) {
  if (!fileHandler->GetProtection() || fileHandler->GetProtection()->AccessCheck(mip::rights::Export())) {
    return;
  }

  throw NoPermissionsError(
      NoPermissionsError::Category::AccessDenied,
      "A minimum right of EXPORT is required to change label or protection",
      fileHandler->GetProtection()->GetProtectionDescriptor()->GetReferrer(),
      fileHandler->GetProtection()->GetOwner());
}

shared_ptr<FileProfile> CreateProfile(
    const shared_ptr<MipContext>& mipContext,
    const shared_ptr<ConsentDelegate>& consentDelegate) {
  const shared_ptr<ProfileObserver> sampleProfileObserver = make_shared<ProfileObserver>();

  FileProfile::Settings profileSettings(
      mipContext,
      CacheStorageType::InMemory,
      consentDelegate,
      sampleProfileObserver);

  auto loadPromise = make_shared<std::promise<shared_ptr<FileProfile>>>();
  auto loadFuture = loadPromise->get_future();
  FileProfile::LoadAsync(profileSettings, loadPromise); // Getting the profile
  return loadFuture.get();
}

vector<mip::LabelFilterType> CreateLabelFiltersFromString(const string& labelFilter) {
  vector<mip::LabelFilterType> retVal;
  auto entries = SplitString(labelFilter, ',');
  for (auto& filter : entries) {
    Trim(filter);
    if (filter == "None") {
      retVal.emplace_back(mip::LabelFilterType::None);
    } else if (filter == "CustomProtection") {
      retVal.emplace_back(mip::LabelFilterType::CustomProtection);
    } else if (filter == "TemplateProtection") {
      retVal.emplace_back(mip::LabelFilterType::TemplateProtection);
    } else if (filter == "DoNotForwardProtection") {
      retVal.emplace_back(mip::LabelFilterType::DoNotForwardProtection);
    } else if (filter == "AdhocProtection") {
      retVal.emplace_back(mip::LabelFilterType::AdhocProtection);
    } else if (filter == "HyokProtection") {
      retVal.emplace_back(mip::LabelFilterType::HyokProtection);
    } else if (filter == "PredefinedTemplateProtection") {
      retVal.emplace_back(mip::LabelFilterType::PredefinedTemplateProtection);
    } else if (filter == "DoubleKeyProtection") {
      retVal.emplace_back(mip::LabelFilterType::DoubleKeyProtection);
    } else if (filter == "DoubleKeyUserDefinedProtection") {
      retVal.emplace_back(mip::LabelFilterType::DoubleKeyUserDefinedProtection);
    } else if (filter == "SensitiveInformationClassifier") {
      retVal.emplace_back(mip::LabelFilterType::SensitiveInformationClassifier);
    } else if (filter == "MachineLearningClassifier") {
      retVal.emplace_back(mip::LabelFilterType::MachineLearningClassifier);
    } else if (filter == "ExtendedSensitiveInformationClassifier") {
      retVal.emplace_back(mip::LabelFilterType::ExtendedSensitiveInformationClassifier);
    } else if (filter.empty()) {
      //Do nothing
    } else {
      throw cxxopts::OptionException(string("Filter type not recognized: ") + filter);
    }
  }
  return retVal;
}

void ConfigureFunctionality(
    FileEngine::Settings& settings,
    const string& enableFunctionality,
    const string& disableFunctionality) {
  for(const auto& filter : CreateLabelFiltersFromString(enableFunctionality)) {
    settings.ConfigureFunctionality(filter, true);
  }

  for(const auto& filter : CreateLabelFiltersFromString(disableFunctionality)) {
    settings.ConfigureFunctionality(filter, false);
  }
}

shared_ptr<FileEngine> GetFileEngine(
    const shared_ptr<FileProfile>& fileProfile,
    const shared_ptr<AuthDelegate>& authDelegate,
    const string& username,
    const string& protectionBaseUrl,
    const string& policyBaseUrl,
    const string& policyPath,
    bool enableMsg,
    bool decryptTopOnly,
    bool enablePowerBI,
    bool protectionOnly,
    const string& locale,
    const string& enableFunctionality,
    const string& disableFunctionality,
    bool keepPdfLinearization) {
  FileEngine::Settings settings(Identity(username), authDelegate, "" /*clientData*/, locale, false /* loadSensitivityTypes*/);

  settings.SetCloud(mip::Cloud::Commercial);
  settings.SetProtectionOnlyEngine(protectionOnly);

  if (!protectionBaseUrl.empty() && !policyBaseUrl.empty()) {
    settings.SetProtectionCloudEndpointBaseUrl(protectionBaseUrl);
    settings.SetPolicyCloudEndpointBaseUrl(policyBaseUrl);
    settings.SetCloud(mip::Cloud::Custom);
  }

  ConfigureFunctionality(settings, enableFunctionality, disableFunctionality);
  
  vector<pair<string, string>> customSettings;
  if (!policyPath.empty()) { // If Policy path was given, saving the policy in custom setting
    customSettings.emplace_back(mip::GetCustomSettingPolicyDataName(), ReadPolicyFile(policyPath)); //Save the content of the policy in custom setting
  }
  if (enableMsg) {
    customSettings.emplace_back(mip::GetCustomSettingEnableMsgFileType(), "true"); // enable msg format for sample application testing.
  }
  if (enablePowerBI) {
    customSettings.emplace_back(mip::GetCustomSettingEnablePowerBIFileType(), "true"); // enable PowerBI format for sample application testing.
  }
  if (keepPdfLinearization) {
    customSettings.emplace_back(mip::GetCustomSettingKeepPdfLinearization(), "true"); // keep pdf linearization for sample application testing.
  }
  if (decryptTopOnly) {
    // decrypt only the top of the message, keep attachment protected.
    customSettings.emplace_back(mip::GetCustomSettingContainerDecryptionOption(),
        mip::ContainerDecryptionOptionString(mip::ContainerDecryptionOption::Top)); 
  }
  settings.SetCustomSettings(customSettings);

  auto addEnginePromise = make_shared<std::promise<shared_ptr<FileEngine>>>();
  auto addEngineFuture = addEnginePromise->get_future();
  fileProfile->AddEngineAsync(settings, addEnginePromise); // Getting the engine
  return addEngineFuture.get();
}

shared_ptr<FileHandler> GetFileHandler(
    const shared_ptr<FileEngine>& fileEngine,
    const shared_ptr<Stream>& stream,
    const string& filePath,
    DataState dataState,
    bool displayClassificationRequests,
    string applicationScenarioId) {
  auto fileExecutionState = make_shared<FileExecutionStateImpl>(dataState, nullptr, displayClassificationRequests, applicationScenarioId);
  auto createFileHandlerPromise = make_shared<std::promise<shared_ptr<FileHandler>>>();
  auto createFileHandlerFuture = createFileHandlerPromise->get_future();
  bool auditDiscoveryEnabled = !displayClassificationRequests;
  // Here content identifier is same as the filePath
  if (stream) {
    fileEngine->CreateFileHandlerAsync(stream, filePath, auditDiscoveryEnabled, make_shared<FileHandlerObserver>(), createFileHandlerPromise, fileExecutionState); // create the file handler
  } else {
    fileEngine->CreateFileHandlerAsync(filePath, filePath, auditDiscoveryEnabled, make_shared<FileHandlerObserver>(), createFileHandlerPromise, fileExecutionState); // create the file handler
  }
  return createFileHandlerFuture.get();
}

string GetWorkingDirectory(int argc, char* argv[]) {
  string fileSamplePath;
#ifdef _WIN32
  if (argc > 0) {
    fileSamplePath = string(argv[0]);
  }
#else 
  UNUSED(argc);
  UNUSED(argv);
#endif // _WIN32

#ifdef __linux__
  char result[MAX_PATH];
  auto count = readlink("/proc/self/exe", result, MAX_PATH);
  fileSamplePath = string(result, (count > 0) ? count : 0);
#endif // __linux__

  fileSamplePath = GetDirFromPath(fileSamplePath);
  return fileSamplePath;
}

} // namespace

int main_impl(int argc, char* argv[]) {
  try {
    const int argCount = argc; // need to save it as cxxopts change it while parsing
    auto fileSampleWorkingDirectory = GetWorkingDirectory(argc, argv);
    auto helpString = kApplicationName + " Version: " + VER_FILE_VERSION_STR;
    cxxopts::Options options("file_sample", helpString);
    options.positional_help("<Extra args>");

    options.add_options()
      // Action choice
      ("f,file", "Path to the file to work on.", cxxopts::value<string>(), "File path")
      ("g,getlabel", "Show the labels and protection that applies on the file.")
      ("s,setlabel", "Set a label with <labelId>. If downgrading label - will apply "
        "<justification message>, if needed and specified.", cxxopts::value<string>())
      ("d,delete", "Delete the current label from the file with <justification message>, if needed and specified.")
      ("p,protect", "Protects the given file with custom permissions, according to given lists of users and rights.")
      ("templateid", "Protect using Template ID", cxxopts::value<string>())
      ("users", "Comma-separated list of users", cxxopts::value<string>())
      ("l,listlabels", "Show all available labels with their ID values.")
      ("u,unprotect", "Remove protection from the given file.")
      ("getpublishinglicense", "Gets raw publishing license from a protected filed")
      ("c,classifierrequests", "Gets the SITs of the classifiers that will be output")
      ("getFileStatus", "Show if the file is protected, labeled or contains protected objects")
      
      // Action-dependent options
      ("standard", "The label will be standard label and will override standard label only.", cxxopts::value<bool>())
      ("privileged", "The label will be privileged label and will override any label.", cxxopts::value<bool>())
      ("auto", "The label will be standard label and will override any label.", cxxopts::value<bool>())
      ("r,rights", "Comma-separated list of rights to users", cxxopts::value<string>())
      ("o,roles", "Comma-separated list of roles to users", cxxopts::value<string>())
      ("x,expiration", "Expiration date (UTC) respresent by ISO 8601 (yyyy-mm-ddThh:mm:ssZ) for custom permissions protection", cxxopts::value<string>())
      ("j,justification", "Justification message to apply with set or remove label.", cxxopts::value<string>())
      
      // Auth options
      ("username", "Set username for authentication.", cxxopts::value<string>())
      ("password", "Set password for authentication.", cxxopts::value<string>())
      ("clientid", "Set ClientID for authentication.", cxxopts::value<string>())
      ("scctoken", "Set authentication token for SCC.", cxxopts::value<string>())
      ("protectiontoken", "Set authentication token for protection.", cxxopts::value<string>())
      ("protectionbaseurl", "Cloud endpoint base url for protection operations (e.g. https://api.aadrm.com)", cxxopts::value<string>())
      ("policybaseurl", "Cloud endpoint base url for policy operations (e.g. https://dataservice.protection.outlook.com)", cxxopts::value<string>())

      // Other options
      ("dataState", "(Optional) Set dataState of content. ['motion'|'use'|'rest'] (Default='rest')", cxxopts::value<string>())	
      ("policy", "Set path for local policy file.", cxxopts::value<string>())
      ("EnableFunctionality", "List of functionality to enable.", cxxopts::value<string>())
      ("DisableFunctionality", "List of functionality to disable.", cxxopts::value<string>())
      ("extendedkey", "Set an extended property key.", cxxopts::value<string>())
      ("extendedvalue", "Set the extended property value.", cxxopts::value<string>())
      ("enableMsg", "Set the experimental msg support.", cxxopts::value<bool>())
      ("enablepbi", "Set the experimental PowerBI support.", cxxopts::value<bool>())
      ("inspect", "Attempt to inspect the file.", cxxopts::value<bool>())
      ("writeattachments", "Write attachments.", cxxopts::value<bool>())
      ("decrypttoponly", "In container decyrption decrypt top only", cxxopts::value<bool>())
//useStreamApi command prompt
      ("useStreamApi", "Use stream based api.", cxxopts::value<bool>())
      ("keep_pdf_linearization", "Keep pdf linearization", cxxopts::value<bool>())
      ("locale", "Set the locale/language (default 'en-US')", cxxopts::value<string>())
      ("telemetrySettings", "(Advanced) Custom telemetry settings", cxxopts::value<string>())
      ("app_scenario_id", "An identifier which correlates application events with the corresponding protection service REST requests.", cxxopts::value<string>())
      ("featureFlags", "Comma-separated list of pairs <Feature Flag (decimal)>:<True/False/0/1>", cxxopts::value<string>())
      ("offline", "Do not use network access")
      ("protectionOnly", "Set the file engine is protection only.", cxxopts::value<bool>())
      ("h,help", "Print help and exit.")
      ("version", "Display version information.");

    options.parse_positional({ "file", "setlabel", "justification", "username", "password", "policy", "exportpolicy", "app_scenario_id", "featureFlags" });
    options.parse(argc, argv);

    // help
    if (options.count("help") || argCount <= 1) {
      cout << options.help({ "" }) << endl;
      return EXIT_SUCCESS;
    }

    if (options.count("version"))
    {
      cout << VER_FILE_VERSION_STR;
      return EXIT_SUCCESS;
    }

    string locale = "en-US";
    if (options.count("locale"))
      locale = options["locale"].as<string>();

    // Parse auth options
    auto username = options["username"].as<string>();
    auto password = options["password"].as<string>();
    auto clientId = options["clientid"].as<string>();
    
    auto sccToken = options["scctoken"].as<string>();
    auto protectionToken = options["protectiontoken"].as<string>();

    auto protectionBaseUrl = options["protectionbaseurl"].as<string>();
    auto policyBaseUrl = options["policybaseurl"].as<string>();

    auto policyPath = options["policy"].as<string>();
    bool enableMsg = options["enableMsg"].as<bool>();
    bool decryptTopOnly = options["decrypttoponly"].as<bool>();
    bool enablePowerBI = options["enablepbi"].as<bool>();
    bool useStreamApi = options["useStreamApi"].as<bool>();
    bool writeAttachments = options["writeattachments"].as<bool>();
    bool classifierrequests = options["classifierrequests"].as<bool>();
    bool keepPdfLinearization = options["keep_pdf_linearization"].as<bool>();
    bool offlineFlag = options["offline"].as<bool>();
    const auto protectionOnly = options.count("protectionOnly") || options.count("protect") || options.count("templateid") || options.count("getpublishinglicense");
    //Authentication:
    const auto hasAuthentication = (!username.empty() && !password.empty()) || (!protectionToken.empty() && (protectionOnly || !sccToken.empty()));

    auto enableFunctionality = options["EnableFunctionality"].as<string>();
    auto disableFunctionality = options["DisableFunctionality"].as<string>();

    auto applicationScenarioId = options["app_scenario_id"].as<string>();
    
    map<string, string> telemetrySettings;
    if (options.count("telemetrySettings"))
      telemetrySettings = SplitDict(options["telemetrySettings"].as<string>());
    
    if (!username.empty() && !password.empty() && !sccToken.empty() && !protectionToken.empty()) {
      cout << "Only one authentication method supported. Please pass username and password or tokens";
      return EXIT_SUCCESS;
    }

    if (!hasAuthentication && policyPath.empty() && !options.count("getFileStatus")) {
      cout << "No authentication and no policy path was found, and not file non-network operations.";
      return EXIT_SUCCESS;
    }

    if ((options.count("setlabel") || options.count("protect")) && username.empty() && protectionBaseUrl.empty()) {
      cout << "When applying a label, either username or protectionbaseurl+policybaseurl must be specified";
      return EXIT_SUCCESS;
    }

    if (!options.count("listlabels") && !options.count("file")) {
      cout << "When do file operation, file must be specified";
    }

    auto authDelegate = make_shared<AuthDelegateImpl>(false /*isVerbose*/, username, password, clientId, sccToken, protectionToken, fileSampleWorkingDirectory);
    auto consentDelegate = make_shared<ConsentDelegateImpl>(false /*isVerbose*/);

    // Application ID refers to Azure application ID
    // see https://docs.microsoft.com/en-us/azure/azure-resource-manager/resource-group-create-service-principal-portal
    ApplicationInfo appInfo;
    appInfo.applicationId = "2bdc6418-1745-4c34-8bc9-b471b909b6db";
    appInfo.applicationName = "CAD Standalone";
    appInfo.applicationVersion = "1.0.0.0";

    auto diagnosticOverride = make_shared<mip::DiagnosticConfiguration>();
    diagnosticOverride->customSettings = telemetrySettings;
    auto mipConfiguration = make_shared<mip::MipConfiguration>(appInfo, "file_sample_storage", mip::LogLevel::Trace, offlineFlag);
    mipConfiguration->SetDiagnosticConfiguration(diagnosticOverride);
    map<mip::FlightingFeature, bool> featureSettingsOverride;
    if (options.count("featureFlags")) {
      featureSettingsOverride = SplitFeatures(options["featureFlags"].as<string>());
    }
    mipConfiguration->SetFeatureSettings(featureSettingsOverride);

    auto mipContext = MipContext::Create(mipConfiguration);
    sample::ShutdownManager lifetimeManager(mipContext);

    // file
    string filePath;
    shared_ptr<mip::Stream> fileStream = nullptr;
    if (options.count("file")) 
    {
      filePath = options["file"].as<string>();
      if (filePath.empty()) 
      {
        cout << options.help({""}) << endl;
        return EXIT_SUCCESS;
      }
      if (useStreamApi) 
      {
        fileStream = GetInputStreamFromFilePath(filePath);
      } 
    }

    // get file status
    if (options.count("getFileStatus") || options.count("getlabel")) {
      auto fileStatus = GetFileStatus(filePath, fileStream, mipContext);
      if (fileStatus->IsProtected()) {
        cout << "File is protected" << endl;
      } 
      if (fileStatus->IsLabeled()) {
        cout << "File is labeled" << endl;
      } 
      if (fileStatus->ContainsProtectedObjects()) {
        cout << "File contains protected objects" << endl;
      }  
      if (options.count("getFileStatus")) {
        return EXIT_SUCCESS;
      }
    }

    auto profile = CreateProfile(mipContext, consentDelegate);
    
    auto fileEngine = GetFileEngine(
        profile,
        authDelegate,
        username,
        protectionBaseUrl,
        policyBaseUrl,
        policyPath, 
        enableMsg,
        decryptTopOnly,
        enablePowerBI,
        protectionOnly, 
        locale,
        enableFunctionality,
        disableFunctionality,
        keepPdfLinearization);

    // listlabels
    if (options.count("listlabels")) {
      auto labels(fileEngine->ListSensitivityLabels());
      ListLabels(labels);
      return EXIT_SUCCESS;
    }

    DataState dataState = DataState::REST;
    string dataStateString = options["dataState"].as<string>();
    if (!dataStateString.empty()) {
      if (dataStateString == "motion") {
        dataState = DataState::MOTION;
      } else if (dataStateString == "use") {
        dataState = DataState::USE;
      } else if (dataStateString == "rest") {
        dataState = DataState::REST;
      } else {	
        throw std::invalid_argument("Unknown DataState parameter");	
      }	
    }

    // All the rest of commands are file based. We need file handler.
    auto fileHandler = GetFileHandler(fileEngine, fileStream, filePath, dataState, classifierrequests, applicationScenarioId);

    if (options.count("getlabel")) {
      GetLabel(fileHandler);
      return EXIT_SUCCESS;
    }

    // inspect
    if (options.count("inspect")) {
      Inspect(mipContext, fileStream, fileHandler, filePath, writeAttachments);
      return EXIT_SUCCESS;
    }

    //auto method = AssignmentMethod::STANDARD;
    //if (options["auto"].as<bool>()) {
      auto method = AssignmentMethod::AUTO;
    /*} else if (options["privileged"].as<bool>()) {
      method = AssignmentMethod::PRIVILEGED;
    }*/

    // setlabel
    if (options.count("setlabel")) {
      //Parse set label options
      const auto labelId = options["setlabel"].as<string>();
      string justificationMessage;
      string extendedKey;
      string extendedValue;
      vector<pair<string, string>> extendedProperties;

      if (options.count("justification")) {
        justificationMessage = options["justification"].as<string>();
      }

      if (options.count("extendedkey")) {
        if (options.count("extendedvalue")) {
          extendedKey = options["extendedkey"].as<string>();
          extendedValue = options["extendedvalue"].as<string>();
          pair<string, string> extendedProperty = pair<string, string>(extendedKey, extendedValue);
          extendedProperties.push_back(extendedProperty);
        } else {
          throw cxxopts::OptionException("Missing extendedvalue.");
        }
      }

      EnsureUserHasRights(fileHandler);
      
      //uint8_t buffer[4096] = { 0 };
      //std::streamsize bytesRead = 0;
      vector<uint8_t> outmemoryFile;
      //outmemoryFile.reserve(409600);
      outmemoryFile.resize(409600);

      shared_ptr<mip::Stream> outfileStream = make_shared<StreamOverBuffer>(move(outmemoryFile), 0);
      // Set the label on the file
      //SetLabel(fileHandler, fileEngine->GetLabelById(labelId), filePath, outfileStream, method, justificationMessage, extendedProperties);
      SetLabel(fileHandler, fileEngine->GetLabelById(labelId), filePath, nullptr, method, justificationMessage, extendedProperties);
      return EXIT_SUCCESS;
    }

    // delete
    if (options.count("delete")) {
      string justificationMessage;
      if (options.count("justification")) {
        justificationMessage = options["justification"].as<string>();
      }

      EnsureUserHasRights(fileHandler);
      //uint8_t buffer[4096] = { 0 };
      //std::streamsize bytesRead = 0;
      vector<uint8_t> outmemoryFile;
      //outmemoryFile.reserve(409600);
      outmemoryFile.resize(409600);

      shared_ptr<mip::Stream> outfileStream = make_shared<StreamOverBuffer>(move(outmemoryFile), 0);

      // SetLabel without labelId delete the label
      SetLabel(fileHandler, nullptr, filePath, outfileStream, method, justificationMessage, vector<pair<string, string>>());
      return EXIT_SUCCESS;
    }

    // unprotect
    if (options.count("unprotect")) {
      EnsureUserHasRights(fileHandler);
      Unprotect(mipContext, fileHandler, fileStream, filePath);
      return EXIT_SUCCESS;
    }

    // protect
    if (options.count("protect")) {
      if (options.count("rights") || options.count("roles")) {
        EnsureUserHasRights(fileHandler);
        ProtectWithCustomPermissions(
            fileHandler, 
            options["users"].as<string>(),
            options["rights"].as<string>(),
            options["roles"].as<string>(),
            options["expiration"].as<string>());

        return EXIT_SUCCESS;
      }

      // If protect option was given but no rights were provided throw exception
      throw cxxopts::OptionException("Missing rights for protection. use <rights>.");
    }

    //protect using template ID
    if (options.count("templateid")) {
      EnsureUserHasRights(fileHandler);
      ProtectWithPermissions(fileHandler, ProtectionDescriptorBuilder::CreateFromTemplate(options["templateid"].as<string>()));
      return EXIT_SUCCESS;
    }

    // Get PL
    if (options.count("getpublishinglicense")) {
      const auto& publishingLicense = fileHandler->GetProtection()->GetSerializedPublishingLicense();
      string publishingLicenseStr(publishingLicense.begin(), publishingLicense.end());
      cout << publishingLicenseStr << endl;
      return EXIT_SUCCESS;
    }

    // default when there is a only file path - Show labels
    GetLabel(fileHandler);

  } catch (const cxxopts::OptionException& ex) {
    cout << "Error parsing options: " << ex.what() << endl;
    return EXIT_FAILURE;
  } catch (const std::exception& ex) {
    cout << "Something bad happened: " << ex.what() << "\nExiting." << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#ifdef _WIN32
/*int wmain(int argc, wchar_t* argv[]) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; ++i)
    args.push_back(ConvertWStringToString(argv[i]));
  
  std::unique_ptr<char*[]> ptr(new char*[argc + 1]);
  for (int i = 0; i < argc; ++i)
    ptr[i] = const_cast<char*>(args[i].c_str());
  ptr[argc] = nullptr;

  auto result = main_impl(argc, ptr.get());
  return result;
}*/
#else
int main(int argc, char** argv) {
  auto result = main_impl(argc, argv);
  return result;
}
#endif