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
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Memory/VAllocator.hpp"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <nlohmann/json.hpp>

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

using RootArray = std::vector<PaintNodePtr>;

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
  using FontNameVisitor =
    std::function<void(const std::string& familyName, const std::string& subFamilyName)>;
  static std::vector<PaintNodePtr> build(const json& j, bool resetOrigin = true)
  {
    SceneBuilder builder;
    builder.buildImpl(j, resetOrigin);
    return builder.m_frames;
  }

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
  SceneBuilder setDoc(json doc)
  {
    SET_BUILDER_OPTION(m_doc, doc);
  };

  SceneBuilder setFontNameVisitor(FontNameVisitor visitor)
  {
    SET_BUILDER_OPTION(m_fontNameVisitor, visitor);
  };

  SceneBuilder setAllocator(VAllocator* allocator)
  {
    SET_BUILDER_OPTION(m_alloc, allocator);
  };

  SceneBuilderResult build();

  static std::tuple<glm::mat3, glm::mat3, glm::mat3> fromMatrix(const json& j);
  static Bound fromBound(const json& j, const glm::mat3& totalMatrix);
  static Style fromStyle(const json& j, const Bound& bound, const glm::mat3& totalMatrix);

private:
  std::vector<PaintNodePtr> m_frames;

  std::optional<std::string> m_version;
  bool                       m_invalid{ false };
  bool                       m_resetOrigin{ false };
  std::optional<json>        m_doc;
  VAllocator*                m_alloc{ nullptr };

  FontNameVisitor m_fontNameVisitor;

  void moveToThis(SceneBuilder&& that)
  {
    ASSERT(!that.m_invalid);
    m_frames = std::move(that.m_frames);
    m_version = std::move(that.m_version);
    m_resetOrigin = std::move(that.m_resetOrigin);
    m_doc = std::move(that.m_doc);
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

  PaintNodePtr makeContour(const json& j, const json& parent, const glm::mat3& totalMatrix);

  PaintNodePtr fromFrame(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromImage(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromText(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromPath(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromObject(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromGroup(const json& j, const glm::mat3& totalMatrix);

  std::vector<PaintNodePtr> fromFrames(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromSymbolInstance(const json& j, const glm::mat3& totalMatrix);

  PaintNodePtr fromSymbolMaster(const json& j, const glm::mat3& totalMatrix);

  std::vector<PaintNodePtr> fromSymbolMasters(const json& j, const glm::mat3& totalMatrix);

  std::vector<PaintNodePtr> fromTopLevelFrames(const json& j, const glm::mat3& totalMatrix);

  SceneBuilder() = default;

  void buildImpl(const json& j, bool resetOrigin);

  static json defaultTextAttr();
};

} // namespace VGG::layer
