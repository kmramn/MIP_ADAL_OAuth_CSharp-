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

#ifndef SAMPLE_FILE_EXECUTION_STATE_IMPL_H
#define SAMPLE_FILE_EXECUTION_STATE_IMPL_H

#include "mip/file/file_execution_state.h"

#include <iostream>

class FileExecutionStateImpl final : public mip::FileExecutionState {
public:
  FileExecutionStateImpl(
      mip::DataState dataState, 
      std::shared_ptr<mip::ClassificationResults> simulatedClassificationResults = nullptr,
      bool displayClassificationSITs = false,
      std::string applicationScenarioId = std::string())
    : mDataState(dataState),
      mClassificationResults(simulatedClassificationResults),
      mDisplayClassificationSITs(displayClassificationSITs),
      mApplicationScenarioId(applicationScenarioId) {}

  mip::DataState GetDataState() const override { return mDataState; }

  const std::string GetApplicationScenarioId() const override { return mApplicationScenarioId; }

  std::shared_ptr<mip::ClassificationResults> GetClassificationResults(
      const std::shared_ptr<mip::FileHandler>& /*fileHandler*/,
      const std::vector<std::shared_ptr<mip::ClassificationRequest>>& classificationIds) const override{
    if (mDisplayClassificationSITs) {
      std::cout << "Number of classifiers to use: " << classificationIds.size() << std::endl;
      for (auto it = classificationIds.begin(); it != classificationIds.end(); ++it) {
        std::cout << "\t" << (*it)->GetClassificationId() << " " << (*it)->GetRulePackageId() << std::endl;
      }
    }
    return mClassificationResults;
  }



private:
  mip::DataState mDataState;
  std::shared_ptr<mip::ClassificationResults> mClassificationResults;
  
  bool mDisplayClassificationSITs;
  std::string mApplicationScenarioId;
};

#endif //SAMPLE_FILE_EXECUTION_STATE_IMPL_H