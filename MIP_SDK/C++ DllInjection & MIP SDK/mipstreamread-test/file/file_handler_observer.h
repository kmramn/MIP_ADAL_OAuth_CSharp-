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

#ifndef SAMPLE_FILE_HANDLER_OBSERVER_H_
#define SAMPLE_FILE_HANDLER_OBSERVER_H_

#include "mip/file/file_handler.h"

class FileHandlerObserver final : public mip::FileHandler::Observer {
public:
  void OnCreateFileHandlerSuccess(
      const std::shared_ptr<mip::FileHandler>& fileHandler,
      const std::shared_ptr<void>& context) override;

  void OnCreateFileHandlerFailure(
      const std::exception_ptr& error,
      const std::shared_ptr<void>& context) override;
  
  void OnCommitSuccess(
    bool committed, 
    const std::shared_ptr<void>& context) override;
  
  void OnCommitFailure(
    const std::exception_ptr& error, 
    const std::shared_ptr<void>& context) override;

  void OnClassifySuccess(
    const std::vector<std::shared_ptr<mip::Action>>& actions,
    const std::shared_ptr<void>& context) override;

  void OnClassifyFailure(
    const std::exception_ptr& error, 
    const std::shared_ptr<void>& context) override;

  void OnInspectSuccess(
    const std::shared_ptr<mip::FileInspector>& fileInspector,
    const std::shared_ptr<void>& context) override;

  void OnInspectFailure(
    const std::exception_ptr& error,
    const std::shared_ptr<void>& context) override;

  void OnGetDecryptedTemporaryFileSuccess(
    const std::string& decryptedFilePath, 
    const std::shared_ptr<void>& context) override;

  void OnGetDecryptedTemporaryFileFailure(
    const std::exception_ptr& error, 
    const std::shared_ptr<void>& context) override;
};

#endif //SAMPLE_FILE_HANDLER_OBSERVER_H_