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

#include "Utility/Log.hpp"
#include "Layer/Core/VBound.hpp"

#include <memory>
#include <vector>
#include <algorithm>

namespace VGG::layer
{
class VNode;
using VNodePtr = std::shared_ptr<VNode>;
using VNodeRef = std::weak_ptr<VNode>;

class VNode : public std::enable_shared_from_this<VNode>
{
  Bound                             m_bound;
  uint8_t                           m_state;
  std::vector<std::weak_ptr<VNode>> m_observers;

  template<typename Visitor>
  void visitObservers(Visitor&& v)
  {
    for (auto obs : m_observers)
    {
      v(obs);
    }
  }

protected:
  enum EState
  {
    INVALIDATE = 1 << 0
  };
  virtual Bound onRevalidate() = 0;

  bool isInvalid() const;

  void observe(const VNodePtr& sender);

  void unobserve(const VNodePtr& sender);

public:
  VNode() = default;

  void invalidate();

  const Bound& revalidate();

  const Bound& bound() const
  {
    ASSERT(!isInvalid());
    return m_bound;
  }
};
} // namespace VGG::layer
