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

#include "UIGestureRecognizer.hpp"

#include "Types.hpp"

#include "Domain/Layout/Rect.hpp"

#include <chrono>
#include <functional>

namespace VGG::UIKit
{

class UIPanGestureRecognizer : public UIGestureRecognizer
{
public:
  using TPanHandler = std::function<void(UIPanGestureRecognizer&)>;

private:
  double m_lastMovementTime;
  Point  m_lastDelta;

  Point m_translation;
  Point m_velocity;

  TPanHandler m_handler;

public:
  void setHandler(TPanHandler handler)
  {
    m_handler = handler;
  }

  void touchesBegan(UEvent e) override;
  void touchesMoved(UEvent e) override;
  void touchesEnded(UEvent e) override;

  Point translation();
  void  setTranslation(Point translation);
  Point velocity();

private:
  bool translate(Point delta);
  void setState(EUIGestureRecognizerState state) override;
};

} // namespace VGG::UIKit