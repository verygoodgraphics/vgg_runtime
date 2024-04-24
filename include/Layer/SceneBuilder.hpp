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
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Memory/VAllocator.hpp"

#include "Layer/Model/Serde.hpp"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>

#include <exception>
#include <memory>
#include <optional>
#include <variant>

#define SET_BUILDER_OPTION(container, attr)                                                        \
  ASSERT(!this->m_invalid);                                                                        \
  this->container = std::move(attr);                                                               \
  return std::move(*this);

namespace VGG::layer
{

using namespace nlohmann;

using RootArray = std::vector<FramePtr>;

struct SceneBuilderResult
{
  enum class EResultType
  {
    VERSION_MISMATCH,
    BUILD_FAILD
  };
  std::optional<EResultType> type;
  std::optional<RootArray>   root;
  SceneBuilderResult(std::optional<EResultType> type, std::optional<RootArray> root)
    : type(type)
    , root(std::move(root))
  {
  }
  SceneBuilderResult() = default;
};

class SceneBuilder
{
public:
  using FontNameVisitor = Serde::FontNameVisitor;

  static SceneBuilder builder()
  {
    return SceneBuilder();
  }

  SceneBuilder setCheckVersion(std::string version)
  {
    SET_BUILDER_OPTION(m_version, version);
  }
  SceneBuilder setResetOriginEnable(bool enable)
  {
    SET_BUILDER_OPTION(m_resetOrigin, enable);
  }

  SceneBuilder setFontNameVisitor(FontNameVisitor visitor)
  {
    SET_BUILDER_OPTION(m_fontNameVisitor, visitor);
  };

  SceneBuilder setAllocator(VAllocator* allocator)
  {
    SET_BUILDER_OPTION(m_alloc, allocator);
  };

  template<typename M>
  SceneBuilderResult build(std::vector<typename M::Model> objects)
  {
    SceneBuilderResult result;

    glm::mat3 mat = glm::identity<glm::mat3>();
    mat = glm::scale(mat, glm::vec2(1, -1));
    Serde::Context ctx;
    ctx.alloc = m_alloc;
    ctx.fontNameVisitor = m_fontNameVisitor;
    m_frames = Serde::from<M>(objects, mat, ctx);

    if (!m_frames.empty())
    {
      result.root = RootArray();
      for (auto& p : m_frames)
      {
        auto frame = makeFramePtr(std::move(p));
        if (m_resetOrigin)
          frame->resetToOrigin(true);
        result.root->emplace_back(frame);
      }
    }
    m_invalid = true;
    return result;
  }

private:
  std::vector<PaintNodePtr> m_frames;

  std::optional<std::string> m_version;
  bool                       m_invalid{ false };
  bool                       m_resetOrigin{ false };
  VAllocator*                m_alloc{ nullptr };

  FontNameVisitor m_fontNameVisitor;

  void moveToThis(SceneBuilder&& that)
  {
    ASSERT(!that.m_invalid);
    m_frames = std::move(that.m_frames);
    m_version = std::move(that.m_version);
    m_resetOrigin = std::move(that.m_resetOrigin);
    m_alloc = std::move(that.m_alloc);
    m_invalid = std::move(that.m_invalid);
    m_fontNameVisitor = std::move(that.m_fontNameVisitor);
    that.m_invalid = true;
  }

  SceneBuilder(const SceneBuilder&) = delete;
  SceneBuilder& operator=(const SceneBuilder&) = delete;

  SceneBuilder(SceneBuilder&& other) noexcept
  {
    moveToThis(std::move(other));
  }

  SceneBuilder& operator=(SceneBuilder&& other) noexcept
  {
    moveToThis(std::move(other));
    return *this;
  }

  SceneBuilder() = default;
};

} // namespace VGG::layer
