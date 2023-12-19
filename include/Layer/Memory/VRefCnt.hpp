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
#include <cstddef>
namespace VGG::layer
{
class VObject;
class VRefCnt
{
public:
  virtual size_t   ref() = 0;
  virtual size_t   deref() = 0;
  virtual size_t   refCount() const = 0;
  virtual size_t   weakRef() = 0;
  virtual size_t   weakDeref() = 0;
  virtual size_t   weakRefCount() const = 0;
  virtual VObject* object() = 0;
  virtual ~VRefCnt() = default;
};

} // namespace VGG::layer
