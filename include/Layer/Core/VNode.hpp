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
#include "Utility/Log.hpp"
#include "Layer/Core/VBounds.hpp"

#include "Layer/Memory/Ref.hpp"

#include <memory>
#include <vector>
#include <algorithm>

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

class VNode : public ObjectImpl<VObject>
{
  Bounds                m_bounds;
  uint8_t               m_state{ 0 };
  std::vector<VNodeRef> m_observers;

  template<typename Visitor>
  void visitObservers(Visitor&& v)
  {
    for (auto& obs : m_observers)
    {
      v(obs);
    }
  }

protected:
  enum EState
  {
    INVALIDATE = 1 << 0
  };
  virtual Bounds onRevalidate() = 0;

  bool isInvalid() const;

  void observe(VNodePtr sender);

  void unobserve(VNodePtr sender);

public:
  VNode(VRefCnt* cnt, EState initState = (EState)0)
    : ObjectImpl<VObject>(cnt)
    , m_state(initState)
  {
  }

  void invalidate();

  const Bounds& revalidate();

  const Bounds& bounds() const
  {
    ASSERT(!isInvalid());
    return m_bounds;
  }
};
} // namespace VGG::layer
