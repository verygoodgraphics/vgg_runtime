/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "VRefCnt.hpp"
#include <cstdint>
#include <atomic>
#include <utility>
namespace VGG::layer
{

template<typename Base>
class RefCountedObject : public Base
{

  template<typename ObjectType, typename Allocator>
  friend class RefCounterImpl;

  template<typename ObjectType, typename Allocator>
  friend class VNew;

  template<typename ObjectType, typename Allocator>
  friend class RefCounterImplUnsafe;

  template<typename ObjectType, typename Allocator>
  friend class VNewUnsafe;

public:
  template<typename... Args>
  RefCountedObject(VRefCnt* cnt, Args&&... args) noexcept
    : Base(std::forward<Args>(args)...)
    , m_refCnt(cnt)
  {
  }

  template<typename... Args>
  RefCountedObject(Args&&... args) noexcept
    : Base(std::forward<Args>(args)...)
  {
  }
  virtual ~RefCountedObject()
  {
  }

  inline virtual VRefCnt* refCnt() const override final
  {
    return m_refCnt;
  }

  inline virtual size_t ref() override final
  {
    return m_refCnt->ref();
  }

  inline virtual size_t deref() override final
  {
    return m_refCnt->deref();
  }

protected:
  void operator delete(void* ptr)
  {
    delete[] reinterpret_cast<uint8_t*>(ptr);
  }

  template<typename ObjectAllocatorType>
  void operator delete(
    void*                ptr,
    ObjectAllocatorType& allocator,
    const char*          dbgDescription,
    const char*          dbgFileName,
    const uint32_t       dbgLineNumber)
  {
    return allocator.dealloc(ptr);
  }

private:
  friend class RefCountersImpl;

  void* operator new(size_t size)
  {
    return new uint8_t[size];
  }

  template<typename ObjectAllocatorType>
  void* operator new(
    size_t               size,
    ObjectAllocatorType& allocator,
    const char*          dbgDescription,
    const char*          dbgFileName,
    const uint32_t       dbgLineNumber)
  {
    return allocator.alloc(size);
  }
  VRefCnt* const m_refCnt;
};
} // namespace VGG::layer
