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

#include "Layer/Memory/ObjectImpl.hpp"
#include "Layer/Memory/VObject.hpp"
#include "Layer/Memory/VRefCnt.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Config.hpp"
#include "Layer/StackTrace.hpp"
#include "Utility/Log.hpp"
#include "Layer/Core/VBounds.hpp"

#include "Layer/Memory/Ref.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <queue>

#define VGG_CLASS_MAKE(className)                                                                  \
  template<typename... Args>                                                                       \
  static ::VGG::layer::Ref<className> Make(Args&&... args)                                         \
  {                                                                                                \
    return ::VGG::layer::Ref<className>(                                                           \
      ::VGG::layer::V_NEW_UNSAFE<className>(std::forward<Args>(args)...));                         \
  }

namespace VGG::layer
{
class VNode;

using VNodePtr = Ref<VNode>;
using VNodeRef = WeakRef<VNode>;

class Revalidation
{
public:
  Revalidation() = default;
  Revalidation(const Revalidation&) = delete;
  Revalidation& operator=(const Revalidation&) = delete;

  void emit(const Bounds& bounds, const glm::mat3& ctm, VNode* node = nullptr);

  const Bounds& bounds() const
  {
    return m_bounds;
  }

  auto begin() const
  {
    return m_boundsArray.cbegin();
  }
  auto end() const
  {
    return m_boundsArray.cend();
  }

  const std::vector<Bounds>& boundsArray() const
  {
    return m_boundsArray;
  }

  void reset();

private:
  std::vector<Bounds> m_boundsArray;
  Bounds              m_bounds;
};

class VNode : public ObjectImpl<VObject>
{
protected:
  enum EState
  {
    INVALIDATE = 1 << 0,
    DAMAGE = 1 << 1,
    TRAVERSALING = 1 << 2,
    UPDATE = 1 << 3,
  };
  using EStateT = uint8_t;

  enum EDamageTraitBits
  {
    OVERRIDE_DAMAGE = 1 << 0,
    BUBBLE_DAMAGE = 1 << 1,
  };
  using EDamageTrait = uint8_t;

public:
  VNode(VRefCnt* cnt, EState initState = (EState)0, EDamageTrait trait = 0)
    : ObjectImpl<VObject>(cnt)
    , m_state(initState)
    , m_trait(trait)
  {
  }

  void invalidate(bool damage = true);

  void update();

  const Bounds& revalidate(Revalidation* inv = nullptr, const glm::mat3& ctm = glm::mat3(1.0f));

  const Bounds& bounds() const
  {
    ASSERT(!isInvalid());
    return m_bounds;
  }

#ifdef VGG_LAYER_DEBUG
  std::string dbgInfo;
#endif

protected:
#ifdef VGG_LAYER_DEBUG
  virtual int depth() const
  {
    return 0;
  }
#endif
  virtual Bounds onRevalidate(
    Revalidation*    inv = nullptr,
    const glm::mat3& ctm = glm::mat3(1.0f)) = 0;

  bool isInvalid() const;

  void observe(VNodePtr sender);

  void unobserve(VNodePtr sender);

private:
  class ScopedState;
  friend class EventManager;
  Bounds                m_bounds;
  uint8_t               m_state : 4 { 0 };
  const EDamageTrait    m_trait : 2 { 0 };
  std::vector<VNodeRef> m_observers;

  template<typename Visitor>
  void visitObservers(Visitor&& v)
  {
    for (auto& obs : m_observers)
    {
      v(obs);
    }
  }

  static std::queue<VNode*> s_repaintQueue;
};
} // namespace VGG::layer
