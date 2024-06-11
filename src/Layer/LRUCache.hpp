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

#include "Utility/Log.hpp"

#include <unordered_map>
#include <list>

namespace VGG::layer
{
template<typename K, typename V>
class LRUCache
{
private:
  struct Entry
  {
    Entry(const K& key, V&& value)
      : key(key)
      , value(std::move(value))
    {
    }
    K key;
    V value;
    ~Entry()
    {
    }
  };

public:
  using Value = V;
  explicit LRUCache(int maxCount)
    : m_maxCount(maxCount)
  {
  }

  LRUCache(const LRUCache&) = delete;
  LRUCache& operator=(const LRUCache&) = delete;

  V* find(const K& key)
  {
    if (auto it = m_map.find(key); it != m_map.end())
    {
      if (m_lru.begin() != it->second)
      {
        m_lru.splice(m_lru.begin(), m_lru, it->second); // move to head
      }
      Entry* entry = *(it->second);
      return &entry->value;
    }
    return nullptr;
  }

  V* insert(const K& key, V value)
  {
    ASSERT(m_map.find(key) == m_map.end());
    Entry* entry = new Entry(key, std::move(value));
    m_lru.push_front(entry);
    m_map[key] = m_lru.begin();
    while ((int)m_map.size() > m_maxCount)
    {
      if (!m_lru.empty())
      {
        this->remove(std::prev(m_lru.end()));
      }
    }
    return &(entry->value);
  }

  V* insertOrUpdate(const K& key, V value)
  {
    if (auto it = m_map.find(key); it != m_map.end())
    {
      auto entry = *(it->second);
      entry->value = std::move(value);
      return &(entry->value);
    }
    else
    {
      return this->insert(key, std::move(value));
    }
  }

  int count() const
  {
    return m_map.count();
  }

  void purge()
  {
    m_map.clear();
    for (auto it = m_lru.begin(); it != m_lru.end(); ++it)
    {
      delete *it;
    }
    m_lru.clear();
  }

  void remove(const K& key)
  {
    if (auto it = m_map.find(key); it != m_map.end())
    {
      delete *it;
      m_lru.erase(it);
      m_map.erase(key);
      ASSERT(key == it->key);
    }
  }

  ~LRUCache()
  {
    purge();
  }

  auto begin()
  {
    return m_lru.begin();
  };

  auto end()
  {
    return m_lru.end();
  };

private:
  using LRUListType = std::list<Entry*>;
  using LRUMapType = std::unordered_map<K, typename LRUListType::iterator>;
  LRUMapType  m_map;
  LRUListType m_lru;
  int         m_maxCount;

  void remove(typename LRUListType::iterator it)
  {
    m_map.erase((*it)->key);
    delete *it; // delete the entry wrapper
    m_lru.erase(it);
  }
};
} // namespace VGG::layer
