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

#include "Event/Event.hpp"

namespace VGG::UIKit
{

enum class EUIGestureRecognizerState
{
  POSSIBLE,
  BEGAN,
  CHANGED,
  ENDED,
  CANCELLED,
  FAILED,
  RECOGNIZED = ENDED
};

class UIGestureRecognizer
{
protected:
  EUIGestureRecognizerState m_state;

public:
  virtual ~UIGestureRecognizer() = default;

  // virtual void setView() = 0;
  // virtual void abort() = 0;

  auto state()
  {
    return m_state;
  }

  virtual void touchesBegan(UEvent e) = 0;
  virtual void touchesMoved(UEvent e) = 0;
  virtual void touchesEnded(UEvent e) = 0;
  // virtual void touchesCancelled() = 0;
};

} // namespace VGG::UIKit