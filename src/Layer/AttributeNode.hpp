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

#include "Layer/Core/VNode.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Memory/VNewUnsafe.hpp"

#define VGG_ATTRIBUTE(attrName, attrType, attrContainer)                                           \
  const attrType& get##attrName() const                                                            \
  {                                                                                                \
    return attrContainer;                                                                          \
  }                                                                                                \
  void set##attrName(const attrType& v)                                                            \
  {                                                                                                \
    if (attrContainer == v)                                                                        \
      return;                                                                                      \
    attrContainer = v;                                                                             \
    this->invalidate();                                                                            \
  }                                                                                                \
  void set##attrName(attrType&& v)                                                                 \
  {                                                                                                \
    attrContainer = std::move(v);                                                                  \
    this->invalidate();                                                                            \
  }

#define VGG_ATTRIBUTE_PTR(attrName, attrType, attrContainer)                                       \
  attrType* get##attrName() const                                                                  \
  {                                                                                                \
    return attrContainer;                                                                          \
  }                                                                                                \
  void set##attrName(attrType* v)                                                                  \
  {                                                                                                \
    if (attrContainer == v)                                                                        \
      return;                                                                                      \
    attrContainer = v;                                                                             \
    this->invalidate();                                                                            \
  }

#define VGG_CLASS_MAKE(className)                                                                  \
  template<typename... Args>                                                                       \
  static Ref<className> Make(Args&&... args)                                                       \
  {                                                                                                \
    return Ref<className>(V_NEW_UNSAFE<className>(std::forward<Args>(args)...));                   \
  }                                                                                                \
  template<typename... Args>                                                                       \
  static Ref<className> MakeAlloc(VAllocator* alloc, Args&&... args)                               \
  {                                                                                                \
    return Ref<className>(V_NEW_UNSAFE<className>(alloc, std::forward<Args>(args)...));            \
  }

namespace VGG::layer
{

class Renderer;
class Attribute : public VNode
{
public:
  Attribute(VRefCnt* cnt)
    : VNode(cnt)
  {
  }
  virtual void render(Renderer* renderer){};

private:
};

} // namespace VGG::layer
