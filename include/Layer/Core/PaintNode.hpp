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
#include "Layer/Core/VNode.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Config.hpp"

#include <memory>
#include <functional>
#include <optional>
#include <string>
#include <any>

namespace VGG::layer
{
class Renderer;
class ShapeAttribute;
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

enum ERenderTraitBits : uint8_t
{
  RT_RENDER_SELF = 1,
  RT_RENDER_CHILDREN = 1 << 1,
  RT_DEFAULT = RT_RENDER_SELF | RT_RENDER_CHILDREN,
};
using ERenderTrait = uint8_t;

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

class VGG_EXPORTS PaintNode : public VNode
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

public:
  using ChildContainer = std::vector<PaintNodePtr>;

  PaintNode(
    VRefCnt*           cnt,
    int                uniqueID,
    const std::string& name,
    EObjectType        type,
    const std::string& guid,
    ERenderTrait       renderTrait,
    bool               initBase = true);

  int uniqueID() const;

  void addChild(const PaintNodePtr node);

  void addChild(ChildContainer::const_iterator pos, PaintNodePtr node);

  void addSubShape(PaintNodePtr node, EBoolOp op)
  {
    if (!node)
      return;
    node->setClipOperator(op);
    addChild(node);
  }

  PaintNodePtr removeChild(ChildContainer::iterator pos);

  void removeChild(PaintNodePtr node);

  auto rbegin()
  {
    return m_children.rbegin();
  }

  auto begin()
  {
    return m_children.begin();
  }

  auto cbegin() const
  {
    return m_children.cbegin();
  }

  auto end()
  {
    return m_children.end();
  }

  auto rend()
  {
    return m_children.rend();
  }

  auto cend() const
  {
    return m_children.cend();
  }

  PaintNodePtr parent() const
  {
    return !m_parent.expired() ? m_parent.lock() : nullptr;
  }

  const ChildContainer& children()
  {
    return m_children;
  }

  void setOverflow(EOverflow overflow);

  EOverflow overflow() const;

  // ContextSetting& contextSetting();

  void setClipOperator(EBoolOp op);

  void setChildWindingType(EWindingType rule);

  EWindingType childWindingType() const;

  void setFrameBounds(const Bounds& bounds);

  const Bounds& frameBounds() const; // content bound

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

  const std::string& guid() const;

  const std::string& name() const;

  void setMaskBy(std::vector<std::string> masks);

  void setAlphaMaskBy(std::vector<AlphaMask> masks);

  EMaskType maskType() const;

  void setMaskType(EMaskType type);

  void setMaskShowType(EMaskShowType type);

  void setContourOption(ContourOption option);

  void setContourData(ContourData contour);

  const ContourOption& maskOption() const;

  virtual VShape asVisualShape(const Transform* transform);

#ifdef VGG_LAYER_DEBUG
  void        debug(Renderer* render);
  std::string dump() const;
#endif

public:
  void                  setVisible(bool visible);
  bool                  isVisible() const;
  void                  setContextSettings(const ContextSetting& settings);
  const ContextSetting& contextSetting() const;

  Accessor* attributeAccessor();

  using EventHandler = std::function<void(ShapeItemAttibuteAccessor*, void*)>;
  void installPaintNodeEventHandler(EventHandler handler);

  struct NodeAtContext
  {
    int       localX{ 0 };
    int       localY{ 0 };
    glm::mat3 ctm;
    void*     userData{ nullptr };
  };
  using NodeVisitor = void (*)(PaintNode*, const NodeAtContext* ctx);
  void nodeAt(int x, int y, NodeVisitor, void* userData);

  ~PaintNode();

protected:
  void                paintChildren(Renderer* renderer);
  void                render(Renderer* renderer);
  virtual void        onPaint(Renderer* renderer);
  virtual void        dispatchEvent(void* event);
  TransformAttribute* transformAttribute();

  void onSetAccessor(std::unique_ptr<Accessor> acc);
  void onSetStyleItem(Ref<StyleItem> item);
  void onSetShapeAttribute(ShapeAttribute* s);

  StyleItem* styleItem();

  VShape         makeBoundsPath();
  virtual VShape makeContourImpl(ContourOption option, const Transform* mat);
  VShape         childPolyOperation() const;
  Bounds         onRevalidate(Invalidator* inv, const glm::mat3 & mat) override;

#ifdef VGG_LAYER_DEBUG
  int depth() const override
  {
    return level;
  }
#endif

private:
  ChildContainer     m_children;
  WeakRef<PaintNode> m_parent;

public:
#ifdef VGG_LAYER_DEBUG
  bool hoverBounds{ false };
  int  level{ 0 };
#endif
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
