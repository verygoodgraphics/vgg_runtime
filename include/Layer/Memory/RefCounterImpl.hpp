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

#include "Layer/Core/VType.hpp"
#include "Layer/Memory/VObject.hpp"
#include "Layer/Memory/VRefCnt.hpp"
#include <algorithm>
#include <atomic>
#include <mutex>

namespace VGG::layer
{
template<typename ManagedObjectType, typename AllocatorType>
class RefCounterImpl : public VRefCnt
{
  template<typename ObjectType, typename Allocator>
  friend class VNew;

  enum class EObjectState
  {
    UNINITIALIZED,
    ALIVE,
    DESTROYED
  };
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
        m_allocator->free(m_obj);
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
    auto cnt = m_cnt--;
    if (cnt == 0)
    {
      // 1. delete managed object

      // The only variables in critical section are m_objectState and weak reference counter
      // So release the lock after accessing them as soon as possible for uncessary overheading
      std::unique_lock<std::mutex> lk(m_mtx);
      lk.lock();
      auto objWrapper =
        reinterpret_cast<ObjectWrapper<ManagedObjectType, AllocatorType>*>(m_objectBuffer);
      m_objectState = EObjectState::DESTROYED;
      auto deleteCnt = m_weakCnt.load() == 0;
      lk.unlock();

      objWrapper->destroy();
      if (deleteCnt)
      {
        // 2. delete self if necessary
        destroy();
      }
    }
    return cnt;
  }

  size_t refCount() const override
  {
    return m_cnt;
  }

  size_t weakRef() override
  {
    return m_weakCnt;
  }

  size_t weakDeref() override
  {
    auto cnt = m_weakCnt--;

    std::unique_lock<std::mutex> lk(m_mtx);
    // Deletes counter itself if and only if weak_ref == 0 and the object state is DESTROYED
    if (cnt == 0 && m_objectState == EObjectState::DESTROYED)
    {
      lk.unlock(); // release lock as soon as possible
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
    if (m_objectState != EObjectState::ALIVE)
      return nullptr;
    std::unique_lock<std::mutex> lk(m_mtx);

    auto cnt = m_cnt++;
    if (m_objectState == EObjectState::ALIVE && cnt > 1)
    {
      lk.unlock();
      auto objectWrapper =
        reinterpret_cast<ObjectWrapper<ManagedObjectType, AllocatorType>*>(m_objectBuffer);
      return objectWrapper->object();
    }
    m_cnt--;
    return nullptr;
  }

private:
  void init(AllocatorType* allocator, ManagedObjectType* obj)
  {
    new (m_objectBuffer) ObjectWrapper<ManagedObjectType, AllocatorType>(obj, allocator);
    m_objectState = EObjectState::ALIVE;
  }

  void destroy()
  {
    delete this;
  }

  std::atomic_size_t m_cnt = { 1 };
  std::atomic_size_t m_weakCnt = { 0 };
  static size_t constexpr BUFSIZE =
    sizeof(ObjectWrapper<ManagedObjectType, AllocatorType>) / sizeof(size_t);
  ;
  size_t       m_objectBuffer[BUFSIZE];
  EObjectState m_objectState{ EObjectState::UNINITIALIZED };
  std::mutex   m_mtx;
};

} // namespace VGG::layer
