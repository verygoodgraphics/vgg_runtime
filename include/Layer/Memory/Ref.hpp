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

#include "Layer/Memory/VObject.hpp"
#include "Layer/Memory/VRefCnt.hpp"

#include <cstddef>
#include <memory>
#include <cassert>

namespace VGG::layer
{

template<typename T>
class Ref final
{
  using ObjectType = VObject;
  template<typename Other>
  friend class Ref;

  class WeakRefRAII
  {
    T*   m_ptr;
    Ref* m_ref;

    WeakRefRAII(const WeakRefRAII&) = delete;
    WeakRefRAII& operator=(const WeakRefRAII&) = delete;
    WeakRefRAII& operator=(WeakRefRAII&&) = delete;

  public:
    WeakRefRAII(Ref& ptr) noexcept
      : m_ptr(static_cast<T*>(ptr))
      , m_ref(std::addressof(ptr))
    {
    }

    WeakRefRAII(WeakRefRAII&& other) noexcept
      : m_ptr(other.m_ptr)
      , m_ref(other.m_ref)
    {
      other.m_ptr = nullptr;
      other.m_ref = nullptr;
    }
    ~WeakRefRAII()
    {
      if (m_ref && static_cast<T*>(*m_ref) != m_ptr)
      {
        m_ref->attach(m_ptr);
      }
    }

    T*& operator*() noexcept
    {
      return m_ptr;
    }
    const T* operator*() const noexcept
    {
      return m_ptr;
    }

    operator T**() noexcept
    {
      return &m_ptr;
    }
    operator const T**() const noexcept
    {
      return &m_ptr;
    }
  };

public:
  WeakRefRAII operator&()
  {
    return WeakRefRAII(*this);
  }

  const WeakRefRAII operator&() const
  {
    return WeakRefRAII(*this);
  }

public:
  explicit Ref()
    : m_ptr(nullptr)
  {
  }

  explicit Ref(T* p)
    : m_ptr(p)
  {
  }

  Ref(std::nullptr_t)
    : m_ptr(nullptr)
  {
  }

  Ref(const Ref& r)
  {
    static_assert(std::is_base_of<VObject, T>::value);
    m_ptr = r.m_ptr;
    if (m_ptr)
    {
      static_cast<ObjectType*>(m_ptr)->ref();
    }
  }

  template<typename U>
  Ref(const Ref<U>& r)
  {
    static_assert(std::is_base_of<VObject, T>::value);
    m_ptr = r.m_ptr;
    if (m_ptr)
    {
      static_cast<ObjectType*>(m_ptr)->ref();
    }
  }

  template<typename U>
  Ref<U> cast()
  {
    return Ref<U>(dynamic_cast<U*>(get()));
  }

  Ref(Ref&& r) noexcept
  {
    m_ptr = r.take();
  }

  Ref& operator=(const Ref& r)
  {
    if (!attach(r.m_ptr))
      return *this;
    if (this->m_ptr)
      this->m_ptr->ref();
    return *this;
  }

  template<
    typename Derived,
    typename = typename std::enable_if<std::is_base_of<T, Derived>::value>::type>
  Ref& operator=(const Ref<Derived>& r)
  {
    if (!attach(r.m_ptr))
      return *this;
    if (this->m_ptr)
      this->m_ptr->ref();
    return *this;
  }

  Ref& operator=(Ref&& r) noexcept
  {
    attach(r.take());
    return *this;
  }

  template<
    typename Derived,
    typename = typename std::enable_if<std::is_base_of<T, Derived>::value>::type>
  Ref& operator=(Ref<Derived>&& r) noexcept
  {
    attach(r.take());
    return *this;
  }

  const T& operator*() const
  {
    return *m_ptr;
  }

  T& operator*()
  {
    return *m_ptr;
  }

  T* operator->() const
  {
    return m_ptr;
  }

  // T* operator->()
  // {
  //   return m_ptr;
  // }

  operator bool() const
  {
    return m_ptr != nullptr;
  }

  operator T*()
  {
    return m_ptr;
  }
  operator const T*() const
  {
    return m_ptr;
  }

  T* get() const
  {
    return m_ptr;
  }

  T* take()
  {
    T* p = m_ptr;
    m_ptr = nullptr;
    return p;
  }

  void reset()
  {
    drop();
    m_ptr = nullptr;
  }

  ~Ref()
  {
    reset();
  }

private:
  bool attach(T* p)
  {
    if (m_ptr != p)
    {
      drop();
      m_ptr = p;
      return true;
    }
    return false;
  }

  void drop()
  {
    if (m_ptr)
    {
      static_cast<ObjectType*>(m_ptr)->deref();
    }
  }

  T* m_ptr{ nullptr };
};

// ==, != operators for two different Ref types with different orders

template<class T, class U>
inline bool operator==(const Ref<T>& a, const Ref<U>& b)
{
  return a.get() == b.get();
}

template<class T, class U>
inline bool operator!=(const Ref<T>& a, const Ref<U>& b)
{
  return a.get() != b.get();
}

template<class T, class U>
inline bool operator==(const Ref<T>& a, U* b)
{
  return a.get() == b;
}

template<class T, class U>
inline bool operator!=(const Ref<T>& a, U* b)
{
  return a.get() != b;
}
template<class T, class U>
inline bool operator==(T* a, const Ref<U>& b)
{
  return a == b.get();
}

template<class T, class U>
inline bool operator!=(T* a, const Ref<U>& b)
{
  return a != b.get();
}

// ==, != for nullptr and Ref<T> with different orders

template<class T>
inline bool operator==(std::nullptr_t a, const Ref<T>& b)
{
  return !b;
}

template<class T>
bool operator==(const Ref<T>& a, std::nullptr_t b)
{
  return b == a;
}

template<class T>
bool operator!=(std::nullptr_t a, const Ref<T>& b)
{
  return !(a == b);
}

template<class T>
bool operator!=(const Ref<T>& a, std::nullptr_t b)
{
  return !(a == b);
}

template<typename T>
class WeakRef
{
  VRefCnt* m_cnt = nullptr;

public:
  WeakRef(T* object = nullptr) noexcept
  {
    if (object)
    {
      m_cnt = object->refCnt();
      m_cnt->weakRef();
    }
  }

  WeakRef(Ref<T> p) noexcept
    : m_cnt(p ? p->refCnt() : nullptr)
  {
    if (m_cnt)
    {
      m_cnt->weakRef();
    }
  }

  WeakRef(const WeakRef& p) noexcept
    : m_cnt(p.m_cnt)
  {
    if (m_cnt)
    {
      m_cnt->weakRef();
    }
  }

  WeakRef(WeakRef&& p) noexcept
    : m_cnt(std::move(p.m_cnt))
  {
    p.m_cnt = nullptr;
  }

  WeakRef& operator=(const WeakRef& p) noexcept
  {
    if (*this == p)
      return *this;
    release();

    m_cnt = p.m_cnt;
    if (m_cnt)
    {
      m_cnt->weakRef();
    }
    return *this;
  }

  WeakRef& operator=(T* object) noexcept
  {
    return operator=(WeakRef(object));
  }

  WeakRef& operator=(Ref<T> p) noexcept
  {
    release();
    m_cnt = p->refCnt();
    assert(m_cnt);
    m_cnt->weakRef();
    return *this;
  }

  WeakRef& operator=(WeakRef&& p) noexcept
  {
    if (*this == p)
      return *this;

    release();
    m_cnt = std::move(p.m_cnt);
    p.m_cnt = nullptr;
    return *this;
  }

  operator bool() const
  {
    return m_cnt != nullptr;
  }

  bool operator==(const WeakRef& other) noexcept
  {
    return m_cnt == other.m_cnt;
  }

  bool operator!=(const WeakRef& other) noexcept
  {
    return m_cnt != other.m_cnt;
  }

  bool expired() const
  {
    return !(m_cnt != nullptr && m_cnt->refCount() > 0);
  }

  // Only for debug use
  VRefCnt* cnt() const
  {
    return m_cnt;
  }

  Ref<T> lock() const
  {
    if (m_cnt)
      return Ref<T>(static_cast<T*>(m_cnt->object()));
    return nullptr;
  }

  void release()
  {
    if (m_cnt)
    {
      m_cnt->weakDeref();
      m_cnt = nullptr;
    }
  }

  ~WeakRef()
  {
    release();
  }
};

} // namespace VGG::layer
