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
#include "AttributeNode.hpp"
#include <core/SkImageFilter.h>
namespace VGG::layer
{

class ImageFilterAttribute : public Attribute
{
public:
  ImageFilterAttribute(VRefCnt* cnt)
    : Attribute(cnt)
  {
  }
  virtual sk_sp<SkImageFilter> getImageFilter() const = 0;
  VGG_CLASS_MAKE(ImageFilterAttribute);

private:
};

} // namespace VGG::layer
