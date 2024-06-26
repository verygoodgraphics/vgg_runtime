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

#include "Layer/Memory/RefCounterImpl.hpp"
#include "Layer/Memory/RefCounterImplUnsafe.hpp"
#include "Layer/Memory/VAllocator.hpp"

#include <memory>
#include <type_traits>

namespace VGG::layer
{

template<typename ObjectType, typename Allocator>
class VNew
{
  Allocator* const m_allocator;

public:
  VNew(Allocator* allocator)
    : m_allocator(allocator)
  {
  }
  VNew(const VNew&) = delete;
  VNew(VNew&&) = delete;
  VNew& operator=(const VNew&) = delete;
  VNew& operator=(VNew&&) = delete;
  template<typename RefCounterType = RefCounterImpl<ObjectType, Allocator>, typename... Args>
  ObjectType* operator()(Args&&... args)
  {
    auto refcnt = std::unique_ptr<RefCounterType>(new RefCounterType());

    ObjectType* obj = nullptr;
    if (m_allocator)
    {
      obj = new (*m_allocator, 0, 0, 0) ObjectType(refcnt.get(), std::forward<Args>(args)...);
    }
    else
    {
      obj = new ObjectType(refcnt.get(), std::forward<Args>(args)...);
    }
    refcnt->init(m_allocator, obj);
    refcnt.release();
    return obj;
  }
};

// NOLINTBEGIN
template<
  typename Type,
  typename AllocatorType,
  typename... Args,
  typename = typename std::enable_if<std::is_base_of<VAllocator, AllocatorType>::value>::type>
Type* V_NEW(AllocatorType* alloc, Args&&... args)
{
  return VNew<Type, AllocatorType>(alloc)(std::forward<Args>(args)...);
}

template<typename Type, typename... Args>
Type* V_NEW(Args&&... args)
{
  return VNew<Type, VAllocator>(nullptr)(std::forward<Args>(args)...);
}

template<
  typename Type,
  typename AllocatorType,
  typename... Args,
  typename = typename std::enable_if<std::is_base_of<VAllocator, AllocatorType>::value>::type>
Type* V_NEW_UNSAFE(AllocatorType* alloc, Args&&... args)
{
  return VNew<Type, AllocatorType>(alloc)
    .template operator()<RefCounterImplUnsafe<Type, AllocatorType>>(std::forward<Args>(args)...);
}

template<typename Type, typename... Args>
Type* V_NEW_UNSAFE(Args&&... args)
{
  return VNew<Type, VAllocator>(nullptr)
    .template operator()<RefCounterImplUnsafe<Type, VAllocator>>(std::forward<Args>(args)...);
}

// NOLINTEND

} // namespace VGG::layer
