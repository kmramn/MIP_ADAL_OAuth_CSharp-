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

#include "file_handler_observer.h"

using std::exception_ptr;
using std::promise;
using std::shared_ptr;
using std::string;
using std::vector;
using mip::ContentLabel;
using mip::FileHandler;
using mip::FileInspector;

void FileHandlerObserver::OnCreateFileHandlerSuccess(
    const shared_ptr<FileHandler>& fileHandler, 
    const shared_ptr<void>& context) {
  auto fileHandlerPromise = static_cast<promise<shared_ptr<FileHandler>>*>(context.get());
  fileHandlerPromise->set_value(fileHandler);
}

void FileHandlerObserver::OnCreateFileHandlerFailure(
    const exception_ptr& error, 
    const shared_ptr<void>& context) {
  auto fileHandlerPromise = static_cast<promise<shared_ptr<FileHandler>>*>(context.get());
  fileHandlerPromise->set_exception(error);
}

void FileHandlerObserver::OnCommitSuccess(bool committed, const shared_ptr<void>& context) {
  auto onCommitPromise = static_cast<promise<bool>*>(context.get());
  onCommitPromise->set_value(committed);
}

void FileHandlerObserver::OnCommitFailure(const exception_ptr& error, const shared_ptr<void>& context) {
  auto onCommitPromise = static_cast<promise<bool>*>(context.get());
  onCommitPromise->set_exception(error);
}

void FileHandlerObserver::OnClassifySuccess(
    const vector<shared_ptr<mip::Action>>& actions, const std::shared_ptr<void>& context) {
  auto onClassifyPromise = static_cast<promise<vector<shared_ptr<mip::Action>>>*>(context.get());
  onClassifyPromise->set_value(actions);
}

void FileHandlerObserver::OnClassifyFailure(const exception_ptr& error, const shared_ptr<void>& context) {
  auto onClassify = static_cast<promise<vector<shared_ptr<mip::Action>>>*>(context.get());
  onClassify->set_exception(error);
}

void FileHandlerObserver::OnInspectSuccess(
    const std::shared_ptr<FileInspector>& fileInspector,
    const std::shared_ptr<void>& context) {
  auto onInspect = static_cast<promise<shared_ptr<FileInspector>>*>(context.get());
  onInspect->set_value(fileInspector);
}

void FileHandlerObserver::OnInspectFailure(
    const std::exception_ptr& error,
    const std::shared_ptr<void>& context) {
  auto onInspect = static_cast<promise<shared_ptr<FileInspector>>*>(context.get());
  onInspect->set_exception(error);
}

void FileHandlerObserver::OnGetDecryptedTemporaryFileSuccess(
    const string& decryptedFilePath, const std::shared_ptr<void>& context) {
  auto onGetDecrypted = static_cast<promise<string>*>(context.get());
  onGetDecrypted->set_value(decryptedFilePath);
}

void FileHandlerObserver::OnGetDecryptedTemporaryFileFailure(
    const std::exception_ptr& error, const std::shared_ptr<void>& context) {
  auto onGetDecrypted = static_cast<std::promise<string>*>(context.get());
  onGetDecrypted->set_exception(error);
}