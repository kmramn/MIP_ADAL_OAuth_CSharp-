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
#include "stream_over_buffer.h"

#include <cstring>
#include <stdexcept>
#include <vector>

using std::vector;

#ifdef _WIN32
#define MEMCPY(dest, destSize, src, count) memcpy_s(dest, destSize, src, count)
#else // _WIN32
#define MEMCPY(dest, destSize, src, count) memcpy(dest, src, count)
#endif // _WIN32

StreamOverBuffer::StreamOverBuffer(vector<uint8_t>&& buffer, const int64_t end): mBuffer(buffer),mSize(static_cast<uint64_t>(buffer.size())),
      mPosition(0) {
        mEnd = end < 0 ? mSize : end;
}

StreamOverBuffer::~StreamOverBuffer() { }

int64_t StreamOverBuffer::Read(uint8_t* buffer, int64_t bufferLength) {
  auto bytesRead = mPosition + bufferLength <= mSize ? bufferLength : mSize - mPosition;
  if (bytesRead) {
    MEMCPY(buffer, static_cast<size_t>(bufferLength), &mBuffer[static_cast<size_t>(mPosition)], static_cast<size_t>(bytesRead));
    mPosition += bytesRead;
  }
  return bytesRead;
}

int64_t StreamOverBuffer::Write(const uint8_t* buffer, int64_t bufferLength) {
  const auto bytesWritten = mPosition + bufferLength <= mSize ? bufferLength : mSize - mPosition;
  if (bytesWritten) {
    MEMCPY(&mBuffer[static_cast<size_t>(mPosition)], static_cast<size_t>(mSize - mPosition), buffer, static_cast<size_t>(bytesWritten));
    mPosition += bytesWritten;
  }
  if (mPosition > mEnd) {
    mEnd = mPosition;
  }
  return bytesWritten;
}

bool StreamOverBuffer::Flush() { return true; }

void StreamOverBuffer::Seek(int64_t position) {
  if (position > mSize)
      throw std::runtime_error("Position must be smaller than size.");
  mPosition = position;
}

bool StreamOverBuffer::CanRead() const { return true; }

bool StreamOverBuffer::CanWrite() const { return true; }

int64_t StreamOverBuffer::Position() { return mPosition; }

int64_t StreamOverBuffer::Size() {return mEnd; } 

void StreamOverBuffer::Size(int64_t /* value */) { throw std::runtime_error("Not Implemented"); }