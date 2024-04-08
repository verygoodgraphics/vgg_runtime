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

#include "Layer/Memory/VRefCnt.hpp"

namespace VGG::layer
{
template<typename ManagedObjectType, typename AllocatorType>
class RefCounterImplUnsafe : public VRefCnt
{
  template<typename ObjectType, typename Allocator>
  friend class VNew;

  template<typename ObjectType, typename Allocator>
  class ObjectWrapper
  {
  public:
    ObjectWrapper(ObjectType* obj, Allocator* allocator)
      : m_obj(obj)
      , m_allocator(allocator)
    {
    }
    void destroy() const
    {
      if (m_allocator)
      {
        m_obj->~ObjectType();
        m_allocator->dealloc(m_obj);
      }
      else
      {
        delete m_obj;
      }
    }

    ObjectType* object()
    {
      return m_obj;
    }

  private:
    ObjectType* const m_obj = nullptr;
    Allocator* const  m_allocator = nullptr;
  };

public:
  size_t ref() override
  {
    return m_cnt++;
  }

  size_t deref() override
  {
    auto cnt = --m_cnt;
    if (cnt == 0)
    {
      auto objWrapper =
        reinterpret_cast<ObjectWrapper<ManagedObjectType, AllocatorType>*>(m_objectBuffer);
      auto deleteSelf = m_weakCnt == 0;
      objWrapper->destroy();
      if (deleteSelf)
        destroy();
    }
    return cnt;
  }

  size_t refCount() const override
  {
    return m_cnt;
  }

  size_t weakRef() override
  {
    return ++m_weakCnt;
  }

  size_t weakDeref() override
  {
    auto cnt = --m_weakCnt;
    // Deletes counter itself if and only if weak_ref == 0 and the object state is DESTROYED
    if (cnt == 0)
    {
      destroy();
    }
    return cnt;
  }

  size_t weakRefCount() const override
  {
    return m_weakCnt;
  }

  VObject* object() override
  {
    auto cnt = ++m_cnt;
    if (cnt > 1)
    {
      auto objectWrapper =
        reinterpret_cast<ObjectWrapper<ManagedObjectType, AllocatorType>*>(m_objectBuffer);
      return objectWrapper->object();
    }
    --m_cnt;
    return nullptr;
  }

private:
  void init(AllocatorType* allocator, ManagedObjectType* obj)
  {
    new (m_objectBuffer) ObjectWrapper<ManagedObjectType, AllocatorType>(obj, allocator);
  }

  void destroy()
  {
    delete this;
  }

  size_t m_cnt = 1;
  size_t m_weakCnt = 0;
  static size_t constexpr BUFSIZE =
    sizeof(ObjectWrapper<ManagedObjectType, AllocatorType>) / sizeof(size_t);
  size_t m_objectBuffer[BUFSIZE];
};
} // namespace VGG::layer
