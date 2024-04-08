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
#include "Layer/Memory/VNew.hpp"
#include "Layer/Core/VBounds.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Config.hpp"

#include <memory>
#include <functional>
#include <optional>
#include <string>

namespace VGG::layer
{
class Renderer;
class ShapeItemAttibuteAccessor;
class TransformAttribute;
class StyleItem;

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
using PaintNodePtr = VGG::layer::Ref<PaintNode>;
using PaintNodeRef = VGG::layer::WeakRef<PaintNode>;

template<typename... Args>
inline PaintNodePtr makePaintNodePtr(Args&&... args)
{
  return PaintNodePtr(V_NEW<PaintNode>(std::forward<Args>(args)...));
};

class PaintNode__pImpl;
class Accessor;

class VGG_EXPORTS PaintNode : public TreeNode
{
  // some temporary friends
  friend class PaintNodeEventDispatcher;

protected:
  VGG_DECL_IMPL(PaintNode)

  friend class DocBuilder;
  friend class Renderer;
  friend class MaskObject;
  friend class MaskBuilder;
  friend class Frame;

  PaintNode(VRefCnt* cnt, const std::string& name, std::unique_ptr<PaintNode__pImpl> impl);

public:
  PaintNode(
    VRefCnt*           cnt,
    const std::string& name,
    EObjectType        type,
    const std::string& guid,
    bool               initBase = true);
  PaintNode(const PaintNode&) = delete;
  PaintNode& operator=(const PaintNode&) = delete;
  PaintNode(PaintNode&&) = delete;
  PaintNode& operator=(PaintNode&&) = delete;

public:
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

  void setFrameRadius(std::array<float, 4> radius);

  std::array<float, 4> frameRadius() const;

  void setFrameCornerSmoothing(float smooth);

  float frameCornerSmoothing() const;

  void setStyle(const Style& style);

  EBoolOp clipOperator() const;

  Transform mapTransform(const PaintNode* node) const;

  void setTransform(const Transform& transform);

  const Transform& transform() const;

  Transform globalTransform() const;

  const Bounds& frameBounds() const; // content bound

  void setFrameBounds(const Bounds& bounds);

  const std::string& guid() const;

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

public:
  using EventHandler = std::function<void(ShapeItemAttibuteAccessor*, void*)>;
  void installPaintNodeEventHandler(EventHandler handler);
  using NodeVisitor = bool (*)(PaintNode*);
  bool nodeAt(int x, int y, NodeVisitor);

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
  void                paintChildren(Renderer* renderer);
  void                paintSelf(Renderer* renderer);
  void                render(Renderer* renderer);
  virtual void        onPaint(Renderer* renderer);
  virtual void        dispatchEvent(void* event);
  TransformAttribute* transformAttribute();
  Accessor*           attributeAccessor();

  void       onSetAccessor(std::unique_ptr<Accessor> acc);
  void       onSetStyleItem(Ref<StyleItem> item);
  StyleItem* styleItem();

  VShape         makeBoundsPath();
  virtual VShape makeContourImpl(ContourOption option, const Transform* mat);
  VShape         childPolyOperation() const;
  Bounds         onRevalidate() override;
};

class PaintNodeEventDispatcher
{
  PaintNode* m_n;

public:
  PaintNodeEventDispatcher(PaintNode* n)
    : m_n(n)
  {
  }
  void operator()(void* event)
  {
    ASSERT(m_n);
    m_n->dispatchEvent(event);
  }
};
} // namespace VGG::layer
