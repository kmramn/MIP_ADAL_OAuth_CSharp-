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

#include "profile_observer.h"

#include <future>

using std::exception_ptr;
using std::promise;
using std::shared_ptr;
using mip::FileEngine;
using mip::FileProfile;

void ProfileObserver::OnLoadSuccess(const shared_ptr<FileProfile>& profile, const shared_ptr<void>& context) {
  auto profilePromise = static_cast<promise<shared_ptr<FileProfile>> *>(context.get());
  profilePromise->set_value(profile);
}

void ProfileObserver::OnLoadFailure(const exception_ptr& error, const shared_ptr<void>& context) {
  auto profilePromise = static_cast<promise<shared_ptr<FileProfile>> *>(context.get());
  profilePromise->set_exception(error);
 }

void ProfileObserver::OnAddEngineSuccess(const shared_ptr<FileEngine>& engine, const shared_ptr<void>& context) {
  auto profilePromise = static_cast<promise<shared_ptr<FileEngine>> *>(context.get());
  profilePromise->set_value(engine);
}

void ProfileObserver::OnAddEngineFailure(const exception_ptr& error, const shared_ptr<void>& context) {
  auto profilePromise = static_cast<promise<shared_ptr<FileEngine>> *>(context.get());
  profilePromise->set_exception(error);
}
