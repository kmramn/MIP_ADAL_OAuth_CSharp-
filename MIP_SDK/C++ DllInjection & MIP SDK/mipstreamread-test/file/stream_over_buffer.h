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

#ifndef SAMPLE_STREAM_OVER_BUFFER_H_
#define SAMPLE_STREAM_OVER_BUFFER_H_

#include <vector>
#include "mip/stream.h"

class StreamOverBuffer final : public mip::Stream {
public:
  StreamOverBuffer(std::vector<uint8_t>&& memory, const int64_t end = -1);
  ~StreamOverBuffer();
  int64_t Read(uint8_t* buffer, int64_t bufferLength) override;
  int64_t Write(const uint8_t* buffer, int64_t bufferLength) override;
  bool Flush() override;
  void Seek(int64_t position) override;
  bool CanRead() const override;
  bool CanWrite() const override;
  int64_t Position() override;
  int64_t Size() override;
  void Size(int64_t value) override;

private:
  std::vector<uint8_t> mBuffer;
  int64_t mSize;
  int64_t mPosition;
  int64_t mEnd;
};

#endif // SAMPLE_STREAM_OVER_BUFFER_H_