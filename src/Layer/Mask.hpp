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

#include "Layer/Core/VType.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "Effects.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/PaintNode.hpp"

#include <core/SkCanvas.h>
#include <core/SkPictureRecorder.h>
#include <core/SkImageFilter.h>
#include <effects/SkImageFilters.h>

#include <src/shaders/SkPictureShader.h>

#include <optional>
#include <string>

namespace VGG::layer
{

class MaskObject
{
public:
  struct MaskData
  {
    PaintNode*       mask{ nullptr };
    Transform        transform;
    sk_sp<SkBlender> blender{ nullptr };
    MaskData(PaintNode* p, const Transform& t, sk_sp<SkBlender> blender)
      : mask(p)
      , transform(t)
      , blender(std::move(blender))
    {
    }
  };
  std::vector<MaskData>          components;
  std::optional<sk_sp<SkShader>> shader;
};

class MaskBuilder
{
public:
  class MaskIter
  {
  public:
    using Result = std::optional<std::pair<const std::string*, std::optional<EAlphaMaskType>>>;
    virtual Result next() = 0;
    MaskIter() = default;
    MaskIter(const MaskIter&) = delete;
    MaskIter& operator=(const MaskIter&) = delete;
    MaskIter(MaskIter&&) noexcept = default;
    MaskIter& operator=(MaskIter&&) noexcept = default;
    virtual ~MaskIter() = default;
  };

  MaskBuilder() = delete;
  MaskBuilder(const MaskBuilder&) = delete;
  MaskBuilder& operator=(const MaskBuilder&) = delete;
  MaskBuilder(MaskBuilder&&) noexcept = default;
  MaskBuilder& operator=(MaskBuilder&&) noexcept = default;

  static VShape makeShapeMask(
    PaintNode*      self,
    const MaskMap&  maskObjects,
    MaskIter&       iter,
    const SkRect&   rect,
    const SkMatrix* matrix)
  {
    auto   components = collectionMasks(self, maskObjects, iter);
    VShape shape;
    for (const auto& e : components)
    {
      if (shape.isEmpty())
      {
        shape = e.mask->asVisualShape(&e.transform);
      }
      else
      {
        shape.op(e.mask->asVisualShape(&e.transform), EBoolOp::BO_INTERSECTION);
      }
    }
    return shape;
  }

  static sk_sp<SkImageFilter> makeAlphaMaskWith(
    sk_sp<SkImageFilter> input,
    PaintNode*           self,
    const MaskMap&       maskObjects,
    MaskIter&            iter,
    const SkRect&        rect,
    const SkMatrix*      matrix)
  {
    auto alphaMask = makeAlphaMaskFilter(self, maskObjects, iter, rect, matrix);
    return SkImageFilters::Blend(SkBlendMode::kSrcIn, std::move(alphaMask), input, rect);
  }

  static sk_sp<SkImageFilter> makeAlphaMaskFilter(
    PaintNode*      self,
    const MaskMap&  maskObjects,
    MaskIter&       iter,
    const SkRect&   rect,
    const SkMatrix* matrix)
  {
    return SkImageFilters::Shader(makeAlphaMaskShader(self, maskObjects, iter, rect, matrix));
  }

  static sk_sp<SkShader> makeAlphaMaskShader(
    PaintNode*      self,
    const MaskMap&  maskObjects,
    MaskIter&       iter,
    const SkRect&   rect,
    const SkMatrix* matrix);

private:
  struct MaskData
  {
    PaintNode*       mask{ nullptr };
    Transform        transform;
    sk_sp<SkBlender> blender{ nullptr };
    MaskData(PaintNode* p, const Transform& t, sk_sp<SkBlender> blender)
      : mask(p)
      , transform(t)
      , blender(std::move(blender))
    {
    }
  };
  static std::vector<MaskData> collectionMasks(
    PaintNode*     self,
    const MaskMap& maskObjects,
    MaskIter&      iter)
  {
    std::vector<MaskData> components;
    while (auto mask = iter.next())
    {
      const auto id = *mask->first;
      if (id != self->guid())
      {
        if (auto obj = maskObjects.find(id); obj != maskObjects.end())
        {
          if (auto m = obj->second.lock(); m)
          {
            const auto t = m->mapTransform(self);
            components.emplace_back(
              m,
              t,
              m ? (mask->second.has_value() ? getMaskBlender(*mask->second) : nullptr) : nullptr);
          }
        }
        else
        {
          DEBUG("No such mask: %s", id.c_str());
        }
      }
    }
    return components;
  }
};

class AlphaMaskIterator : public MaskBuilder::MaskIter
{
public:
  AlphaMaskIterator(const std::vector<AlphaMask>& masks)
    : m_masks(masks)
    , m_currentIdx(0)
  {
  }

  Result next() override
  {
    if (m_currentIdx < m_masks.size())
    {
      auto& id = m_masks[m_currentIdx];
      m_currentIdx++;
      return std::make_pair(&id.id, id.type);
    }
    return std::nullopt;
  }

private:
  const std::vector<AlphaMask>& m_masks;
  size_t                        m_currentIdx = 0;
};

class ShapeMaskIterator : public MaskBuilder::MaskIter
{
public:
  ShapeMaskIterator(const std::vector<std::string>& masks)
    : m_masks(masks)
    , m_currentIdx(0)
  {
  }

  Result next() override
  {
    if (m_currentIdx < m_masks.size())
    {
      auto& id = m_masks[m_currentIdx];
      m_currentIdx++;
      return std::make_pair(&id, std::nullopt);
    }
    return std::nullopt;
  }

private:
  const std::vector<std::string>& m_masks;
  size_t                          m_currentIdx = 0;
};
} // namespace VGG::layer
