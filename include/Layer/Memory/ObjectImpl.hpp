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

#include "Layer/Memory/VRefCnt.hpp"
#include "Layer/Memory/RefCountedObjectImpl.hpp"
#include "Layer/Memory/RefCounterImpl.hpp"
namespace VGG::layer
{
template<typename Base>
class ObjectImpl : public RefCountedObject<Base>
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
  ObjectImpl(VRefCnt* cnt, Args&&... args)
    : RefCountedObject<Base>(cnt, std::forward<Args>(args)...)
  {
  }
  // Base* acquire() override
  // {
  //   this->ref();
  //   return this;
  // }
};
} // namespace VGG::layer
