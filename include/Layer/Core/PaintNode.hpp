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
// #include "Layer/DocBuilder.hpp"
#include "Math/Geometry.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Core/Node.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Mask.hpp"
#include "Layer/Config.hpp"
#include "Layer/Scene.hpp"

#include <glm/matrix.hpp>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
class SkCanvas;
class SkImageFilter;
class SkImage;

namespace VGG::layer
{
class SkiaRenderer;
class RenderState;

struct ContourOption
{
  ECoutourType contourType{ ECoutourType::MCT_FrameOnly };
  bool         visibilityAware{ true };
  ContourOption(ECoutourType contourType = ECoutourType::MCT_FrameOnly,
                bool         visibilityAware = false)
    : contourType(contourType)
    , visibilityAware(visibilityAware)
  {
  }
};

// NOLINTBEGIN
enum EPaintStrategy
{
  PS_Recursively,
  PS_SelfOnly,
  PS_ChildOnly,
};
// NOLINTEND

struct PaintOption
{
  EPaintStrategy paintStrategy{ PS_Recursively };
  PaintOption(EPaintStrategy paintStrategy = PS_Recursively)
    : paintStrategy(paintStrategy)
  {
  }
};

class PaintNode__pImpl;
class VGG_EXPORTS PaintNode : public Node
{
protected:
  VGG_DECL_IMPL(PaintNode)

protected:
  friend class DocBuilder;
  friend class SkiaRenderer;

private:
  PaintNode(const std::string& name, std::unique_ptr<PaintNode__pImpl> impl);

public:
  PaintNode(const std::string& name, ObjectType type, const std::string& guid);
  PaintNode(const PaintNode&);
  PaintNode& operator=(const PaintNode&) = delete;
  PaintNode(PaintNode&&) = delete;
  PaintNode& operator=(PaintNode&&) = delete;

  virtual NodePtr clone() const override;

  void addChild(const std::shared_ptr<PaintNode> node)
  {
    if (!node)
      return;
    pushChildBack(std::move(node));
  }

  void addSubShape(const std::shared_ptr<PaintNode> node, EBoolOp op)
  {
    if (!node)
      return;
    node->setClipOperator(op);
    addChild(node);
  }

  void setContextSettings(const ContextSetting& settings);

  void setOverflow(EOverflow overflow);

  EOverflow overflow() const;

  const ContextSetting& contextSetting() const;

  ContextSetting& contextSetting();

  void setClipOperator(EBoolOp op);

  void setVisible(bool visible);

  void setChildWindingType(EWindingType rule);

  EWindingType childWindingType() const;

  bool isVisible() const;

  void setStyle(const Style& style);

  Style& style();

  const Style& style() const;

  EBoolOp clipOperator() const;

  glm::mat3 mapTransform(const PaintNode* node) const;

  void setLocalTransform(const glm::mat3& transform);

  const glm::mat3& localTransform() const;

  glm::mat3 worldTransform() const;

  const Bound2& getBound() const;

  void setBound(const Bound2& bound);

  const std::string& guid() const;

  bool isMasked() const;

  void setMaskBy(std::vector<std::string> masks);

  void setAlphaMaskBy(std::vector<AlphaMask> masks);

  EMaskType maskType() const;

  void setMaskType(EMaskType type);

  void setMaskShowType(EMaskShowType type);

  void setContourOption(ContourOption option);

  void setContourData(ContourPtr contour);

  Contour* contour();

  const ContourOption& maskOption() const;

  void setPaintOption(PaintOption option);

  const PaintOption& paintOption() const;

  void setOutlineMask(const Mask& mask);

  virtual Mask asOutlineMask(const glm::mat3* mat);

  ~PaintNode();

public:
  template<typename F>
  void visitFunc(VGG::Node* p, F&& f)
  {
    if (!p)
      return;
    f(static_cast<PaintNode*>(p));
    for (const auto& c : m_firstChild)
    {
      visitFunc(c.get(), std::forward<F>(f));
    }
  }

public:
  // TODO:: chagne the following functions accessbility
  void invokeRenderPass(SkiaRenderer* renderer, int zorder = 0);

protected:
  // Render traverse
  virtual void paintChildrenPass(SkiaRenderer* renderer);
  void         paintChildrenRecursively(SkiaRenderer* renderer);
  virtual void prePaintPass(SkiaRenderer* renderer);
  virtual void postPaintPass(SkiaRenderer* renderer);
  virtual void paintPass(SkiaRenderer* renderer, int zorder = 0);
  void         renderPass(SkiaRenderer* renderer); // TODO:: should be private access
  virtual void paintEvent(SkiaRenderer* renderer);

protected:
  // Mask
  SkPath         makeBoundPath();
  virtual SkPath makeContourImpl(ContourOption option, const glm::mat3* mat);
  SkPath         childPolyOperation() const;
  Mask           makeMaskBy(EBoolOp maskOp, SkiaRenderer* renderer);

protected:
  // Style
  virtual SkPath stylePath();
  void           paintStyle(SkCanvas* canvas, const SkPath& path, const SkPath& mask);

  [[deprecated]] virtual void paintFill(SkCanvas*        canvas,
                                        sk_sp<SkBlender> blender,
                                        const SkPath& path); // TODO:: only for ImageNode overriding
};
} // namespace VGG::layer
