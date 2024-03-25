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
#include "Layer/Memory/VNew.hpp"
#include "Layer/Core/VBound.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Config.hpp"

#include <glm/matrix.hpp>

#include <memory>
#include <optional>
#include <string>
class SkCanvas;
class SkImageFilter;
class SkImage;
class SkBlender;
class Painter;

namespace VGG::layer
{
class Renderer;
class AttributeAccessor;

struct ContourOption
{
  ECoutourType contourType{ ECoutourType::MCT_FRAMEONLY };
  bool         visibilityAware{ true };
  ContourOption(
    ECoutourType contourType = ECoutourType::MCT_FRAMEONLY,
    bool         visibilityAware = false)
    : contourType(contourType)
    , visibilityAware(visibilityAware)
  {
  }
};

enum EPaintStrategy
{
  PS_RECURSIVELY,
  PS_SELFONLY,
  PS_CHILDONLY,
};

struct PaintOption
{
  EPaintStrategy paintStrategy{ PS_RECURSIVELY };
  PaintOption(EPaintStrategy paintStrategy = PS_RECURSIVELY)
    : paintStrategy(paintStrategy)
  {
  }
};

using ShapeData = std::variant<ContourPtr, Ellipse, SkRect, SkRRect>;
using ContourData = std::optional<ShapeData>;

class PaintNode;
#ifdef USE_SHARED_PTR
using PaintNodePtr = std::shared_ptr<PaintNode>;
using PaintNodeRef = std::weak_ptr<PaintNode>;
#else
using PaintNodePtr = VGG::layer::Ref<PaintNode>;
using PaintNodeRef = VGG::layer::WeakRef<PaintNode>;
#endif

template<typename... Args>
inline PaintNodePtr makePaintNodePtr(Args&&... args)
{
#ifdef USE_SHARED_PTR
  auto p = std::make_shared<PaintNode>(nullptr, std::forward<Args>(args)...);
  return p;
#else
  return PaintNodePtr(V_NEW<PaintNode>(std::forward<Args>(args)...));
#endif
};

class PaintNode__pImpl;
class VGG_EXPORTS PaintNode : public TreeNode
{
protected:
  VGG_DECL_IMPL(PaintNode)

protected:
  friend class DocBuilder;
  friend class Renderer;
  friend class MaskObject;
  friend class MaskBuilder;

private:
  PaintNode(VRefCnt* cnt, const std::string& name, std::unique_ptr<PaintNode__pImpl> impl);

public:
  PaintNode(
    VRefCnt*           cnt,
    const std::string& name,
    EObjectType        type,
    const std::string& guid,
    bool               legacyCode = false);
  PaintNode(const PaintNode&) = delete;
  PaintNode& operator=(const PaintNode&) = delete;
  PaintNode(PaintNode&&) = delete;
  PaintNode& operator=(PaintNode&&) = delete;

  void addChild(const PaintNodePtr node)
  {
    if (!node)
      return;
    pushChildBack(std::move(node));
  }

  void addSubShape(PaintNodePtr node, EBoolOp op)
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

  Transform mapTransform(const PaintNode* node) const;

  void setTransform(const Transform& transform);

  const Transform& transform() const;

  // Transform& transform();

  Transform globalTransform() const;

  const Bound& frameBound() const; // content bound

  void setFrameBound(const Bound& bound);

  const std::string& guid() const;

  // bool isMasked() const;

  void setMaskBy(std::vector<std::string> masks);

  void setAlphaMaskBy(std::vector<AlphaMask> masks);

  EMaskType maskType() const;

  void setMaskType(EMaskType type);

  void setMaskShowType(EMaskShowType type);

  void setContourOption(ContourOption option);

  void setContourData(ContourData contour);

  Contour* contour();

  const ContourOption& maskOption() const;

  void setPaintOption(PaintOption option);

  const PaintOption& paintOption() const;

  virtual VShape asVisualShape(const Transform* transform);

  AttributeAccessor* attributeAccessor();

  ~PaintNode();

public:
  template<typename F>
  void visitFunc(VGG::TreeNode* p, F&& f)
  {
    if (!p)
      return;
    f(static_cast<PaintNode*>(p));
    for (const auto& c : m_firstChild)
    {
      visitFunc(c.get(), std::forward<F>(f));
    }
  }

protected:
  // Render traverse
  void         paintChildren(Renderer* renderer);
  void         paintSelf(Renderer* renderer);
  void         render(Renderer* renderer); // TODO:: should be private access
  virtual void paintEvent(Renderer* renderer);

protected:
  // Mask
  VShape         makeBoundPath();
  virtual VShape makeContourImpl(ContourOption option, const Transform* mat);
  VShape         childPolyOperation() const;

  virtual void onDrawAsAlphaMask(Renderer* renderer, sk_sp<SkBlender> blender);
  virtual void onDrawStyle(
    Renderer*        renderer,
    const VShape&    path,
    const VShape&    mask,
    sk_sp<SkBlender> blender);
  Bound onRevalidate() override;

protected:
  virtual Bound onDrawFill(
    Renderer*            renderer,
    sk_sp<SkBlender>     blender,
    sk_sp<SkImageFilter> imageFilter,
    const VShape&        path,
    const VShape&        mask);
};
} // namespace VGG::layer
