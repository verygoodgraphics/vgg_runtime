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
using RootArray = std::vector<std::shared_ptr<PaintNode>>;

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
  std::vector<std::shared_ptr<PaintNode>> m_frames;
  std::vector<std::shared_ptr<PaintNode>> m_symbols;

  std::optional<std::string> m_version;
  bool                       m_invalid{ false };
  bool                       m_resetOrigin{ false };
  std::optional<json>        m_doc;

  void moveToThis(SceneBuilder&& that)
  {
    ASSERT(!that.m_invalid);
    m_frames = std::move(that.m_frames);
    m_symbols = std::move(that.m_symbols);
    m_version = std::move(that.m_version);
    m_resetOrigin = std::move(that.m_resetOrigin);
    m_doc = std::move(that.m_doc);
    m_invalid = std::move(that.m_invalid);
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

  Bound fromBound(const json& j, const glm::mat3& totalMatrix);

  std::tuple<glm::mat3, glm::mat3, glm::mat3> fromMatrix(const json& j);

  Style fromStyle(const json& j, const Bound& bound, const glm::mat3& totalMatrix);

  template<typename F1, typename F2>
  inline std::shared_ptr<PaintNode> makeObjectCommonProperty(
    const json&      j,
    const glm::mat3& totalMatrix,
    F1&&             creator,
    F2&&             override)
  {
    auto obj = creator(std::move(j.value("name", "")), std::move(j.value("id", "")));
    if (!obj)
      return nullptr;
    auto [originalMatrix, newMatrix, inversedNewMatrix] =
      fromMatrix(j.value("matrix", json::array_t{}));
    const auto convertedMatrix = inversedNewMatrix * totalMatrix * originalMatrix;

    obj->setTransform(Transform(newMatrix));
    const auto b = fromBound(j.value("bounds", json::object_t{}), convertedMatrix);
    obj->setFrameBound(b);

    // Pattern point in style are implicitly given by bound, we must supply the points in original
    // coordinates for correct converting
    obj->setStyle(fromStyle(j.value("style", json::object_t{}), b, convertedMatrix));
    obj->style().cornerSmooth = get_opt<float>(j, "cornerSmoothing").value_or(0.f);
    obj->setContextSettings(j.value("contextSettings", ContextSetting()));
    obj->setMaskBy(std::move(j.value("outlineMaskBy", std::vector<std::string>{})));
    obj->setAlphaMaskBy(std::move(j.value("alphaMaskBy", std::vector<AlphaMask>{})));
    const auto maskType = j.value("maskType", EMaskType::MT_None);
    const auto defaultShowType =
      maskType == EMaskType::MT_Outline
        ? MST_Content
        : (maskType == EMaskType::MT_Alpha && false ? MST_Bound : MST_Invisible);
    const auto maskShowType = j.value("maskShowType", defaultShowType);
    obj->setMaskType(maskType);
    obj->setMaskShowType(maskShowType);
    obj->setOverflow(j.value("overflow", EOverflow::OF_Visible));
    obj->setVisible(j.value("visible", true));
    override(obj.get(), convertedMatrix);
    return obj;
  }

  std::shared_ptr<PaintNode> makeContour(
    const json&      j,
    const json&      parent,
    const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromFrame(const json& j, const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromImage(const json& j, const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromText(const json& j, const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromPath(const json& j, const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromObject(const json& j, const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromGroup(const json& j, const glm::mat3& totalMatrix);

  std::vector<std::shared_ptr<PaintNode>> fromFrames(const json& j, const glm::mat3& totalMatrix);

  std::shared_ptr<PaintNode> fromSymbolInstance(const json& j, const glm::mat3& totalMatrix)
  {
    return nullptr;
  }

  std::shared_ptr<PaintNode> fromSymbolMaster(const json& j, const glm::mat3& totalMatrix);

  std::vector<std::shared_ptr<PaintNode>> fromSymbolMasters(
    const json&      j,
    const glm::mat3& totalMatrix);

  void appendSymbolMaster(std::shared_ptr<PaintNode> master)
  {
    m_symbols.push_back(std::move(master));
  }

  std::vector<std::shared_ptr<PaintNode>> fromTopLevelFrames(
    const json&      j,
    const glm::mat3& totalMatrix)
  {
    std::vector<std::shared_ptr<PaintNode>> frames;
    for (const auto& e : j)
    {
      frames.push_back(fromFrame(e, totalMatrix));
    }
    return frames;
  }

  SceneBuilder() = default;

  void buildImpl(const json& j, bool resetOrigin);

  static json defaultTextAttr();

public:
  static std::vector<std::shared_ptr<PaintNode>> build(const json& j, bool resetOrigin = true)
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

  SceneBuilderResult build()
  {
    SceneBuilderResult result;
    if (!m_doc)
      return { SceneBuilderResult::EResultType::BUILD_FAILD, std::nullopt };
    const auto& doc = *m_doc;
    if (auto it = doc.find("version");
        it == doc.end() || (it != doc.end() && m_version && *it != *m_version))
      result.type = SceneBuilderResult::EResultType::VERSION_MISMATCH;
    buildImpl(doc, m_resetOrigin);
    result.root = std::move(m_frames);
    m_invalid = true;
    return result;
  }
};

} // namespace VGG::layer
