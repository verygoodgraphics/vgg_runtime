/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <ostream>
#include <core/SkStream.h>
#include "Utility/Log.hpp"
class SkStdOStream : public SkWStream
{
  std::ostream&          m_os;
  std::ostream::pos_type m_begin{ 0 };

public:
  SkStdOStream(const SkStdOStream&) = delete;
  SkStdOStream& operator=(const SkStdOStream&) = delete;
  SkStdOStream(std::ostream& os)
    : m_os(os)
  {
    m_begin = m_os.tellp();
  }

  virtual bool write(const void* buffer, size_t size) override
  {
    m_os.write((char*)buffer, size);
    return m_os.good();
  }

  virtual size_t bytesWritten() const override
  {
    return m_os.tellp() - m_begin;
  }
};
