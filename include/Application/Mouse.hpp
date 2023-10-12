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

struct SDL_Cursor;

namespace VGG
{

class Mouse
{
public:
  enum class ECursor
  {
    ARROW,     /**< Arrow */
    IBEAM,     /**< I-beam */
    WAIT,      /**< Wait */
    CROSSHAIR, /**< Crosshair */
    WAITARROW, /**< Small wait cursor (or Wait if not available) */
    SIZENWSE,  /**< Double arrow pointing northwest and southeast */
    SIZENESW,  /**< Double arrow pointing northeast and southwest */
    SIZEWE,    /**< Double arrow pointing west and east */
    SIZENS,    /**< Double arrow pointing north and south */
    SIZEALL,   /**< Four pointed arrow pointing north, south, east, and west */
    NO,        /**< Slashed circle or crossbones */
    HAND       /**< Hand */
  };

public:
  virtual ~Mouse() = default;

  virtual void setCursor(ECursor type) = 0;
  virtual void resetCursor() = 0;
};

} // namespace VGG
