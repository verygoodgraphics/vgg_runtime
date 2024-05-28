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
#include "PenNode.hpp"
#include "Effects.hpp"
#include "Layer/Pattern.hpp"
#include "Layer/GlobalSettings.hpp"

namespace
{
using namespace VGG;
} // namespace
namespace VGG::layer
{

void FillPenNode::onMakePaint(SkPaint* paint, const Bounds& bounds) const
{
  populateSkPaint(
    m_fill.type,
    m_fill.contextSettings,
    toSkRect(bounds),
    *paint,
    [&](const Gradient& g, const ContextSetting& st)
    {
      paint->setShader(makeGradientShader(bounds, g));
      paint->setAlphaf(st.opacity);
    },
    [&, this](const Pattern& p, const ContextSetting& st)
    {
      if (!this->m_pattern)
      {
        if (auto a = std::get_if<PatternFit>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
        else if (auto a = std::get_if<PatternFill>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
        else if (auto a = std::get_if<PatternStretch>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
        else if (auto a = std::get_if<PatternTile>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
      }
    });
  if (this->m_pattern)
  {
    if (m_pattern->isValid())
    {
      if (m_pattern->frameCount() == 1)
      {
        paint->setShader(this->m_pattern->shader());
      }
      else
      {
        paint->setShader(this->m_pattern->shader(m_currentFrame++));
        m_currentFrame %= m_pattern->frameCount();
        if (isAnimatedPatternEnabled())
        {
          const_cast<FillPenNode*>(this)->update();
        }
      }
    }
  }
}

Bounds FillPenNode::onRevalidate()
{
  return Bounds();
}

void BorderPenNode::onMakePaint(SkPaint* paint, const Bounds& bounds) const

{
  populateSkPaint(
    m_border,
    toSkRect(bounds),
    *paint,
    [&](const Gradient& g, const ContextSetting& st)
    {
      paint->setShader(makeGradientShader(bounds, g));
      paint->setAlphaf(st.opacity);
    },
    [&, this](const Pattern& p, const ContextSetting& st)
    {
      if (!this->m_pattern)
      {
        if (auto a = std::get_if<PatternFit>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
        else if (auto a = std::get_if<PatternFill>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
        else if (auto a = std::get_if<PatternStretch>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
        else if (auto a = std::get_if<PatternTile>(&p.instance); a)
        {
          this->m_pattern = std::make_unique<ShaderPattern>(bounds, *a);
        }
      }
    });
  if (this->m_pattern)
  {
    if (m_pattern->isValid())
    {
      if (m_pattern->frameCount() == 1)
      {
        paint->setShader(this->m_pattern->shader());
      }
      else
      {
        paint->setShader(this->m_pattern->shader(m_currentFrame++));
        m_currentFrame %= m_pattern->frameCount();
        if (isAnimatedPatternEnabled())
        {
          const_cast<BorderPenNode*>(this)->update();
        }
      }
    }
  }
}

Bounds BorderPenNode::onRevalidate()
{
  return Bounds();
}

} // namespace VGG::layer
