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

#include "Domain/Model/DesignModel.hpp"

using namespace VGG::Model;

Subshape::Subshape(const Subshape& other)
  : booleanOperation{ other.booleanOperation }
  , subshapeClass{ other.subshapeClass }
{
  if (other.subGeometry)
  {
    subGeometry = std::make_shared<SubGeometryType>(*other.subGeometry);
  }
}

Subshape& Subshape::operator=(const Subshape& other)
{
  booleanOperation = other.booleanOperation;
  subshapeClass = other.subshapeClass;
  if (other.subGeometry)
  {
    subGeometry = std::make_shared<SubGeometryType>(*other.subGeometry);
  }
  return *this;
}