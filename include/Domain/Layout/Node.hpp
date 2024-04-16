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

#include "Domain/JsonDocument.hpp"
#include "Rect.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class flexbox_node;
class grid_layout;

namespace VGG
{
namespace Domain
{
class ContourElement;
class Element;
class StateTreeElement;
} // namespace Domain
namespace Model
{
struct Object;
}
namespace Layout
{
struct Matrix;

namespace Internal
{
class AutoLayout;

namespace Rule
{
struct Rule;
}

} // namespace Internal
} // namespace Layout

class LayoutNode : public std::enable_shared_from_this<LayoutNode>
{
  enum class EResizing
  {
    FIX_START_FIX_END,
    FIX_START_FIX_SIZE,
    FIX_START_SCALE,
    FIX_END_FIX_SIZE,
    FIX_END_SCALE,
    SCALE,
    FIX_CENTER_RATIO_FIX_SIZE,
    FIX_CENTER_OFFSET_FIX_SIZE
  };
  enum class EAdjustContentOnResize
  {
    SKIP_GROUP_OR_BOOLEAN_GROUP,
    DISABLED,
    ENABLED,
  };

  std::weak_ptr<LayoutNode>                m_parent;
  std::vector<std::shared_ptr<LayoutNode>> m_children;

  std::shared_ptr<LayoutNode>      m_oldState;
  std::shared_ptr<Domain::Element> m_oldElement;

  std::weak_ptr<Domain::Element> m_element;

  std::shared_ptr<Layout::Internal::AutoLayout> m_autoLayout;
  bool                                          m_needsLayout{ false };
  Layout::Rect                                  m_oldFrame;

  std::optional<Layout::Scalar> m_rightMargin;
  std::optional<Layout::Scalar> m_fixStartWidthRatio;
  std::optional<Layout::Scalar> m_fixEndWidthRatio;

  std::optional<Layout::Scalar> m_bottomMargin;
  std::optional<Layout::Scalar> m_fixStartHeightRatio;
  std::optional<Layout::Scalar> m_fixEndHeightRatio;

public:
  using HitTestHook = std::function<bool(const std::string&)>;

  LayoutNode(std::weak_ptr<Domain::Element> element)
    : m_element{ element }
  {
  }
  virtual ~LayoutNode() = default;

public:
  virtual const LayoutNode* asPage() const
  {
    return this;
  }

  virtual const std::shared_ptr<LayoutNode> parent() const
  {
    return m_parent.lock();
  }

  virtual std::shared_ptr<VGG::Domain::Element> elementNode() const
  {
    return m_element.lock();
  }

public:
  std::shared_ptr<LayoutNode> hitTest(
    const Layout::Point& point,
    const HitTestHook&   hasEventListener);

  void addChild(std::shared_ptr<LayoutNode> child)
  {
    if (!child)
    {
      return;
    }

    child->m_parent = weak_from_this();
    m_children.push_back(child);
  }

  std::vector<std::shared_ptr<LayoutNode>> removeAllChildren();
  void                                     detachChildrenFromFlexNodeTree();

  const std::vector<std::shared_ptr<LayoutNode>>& children() const
  {
    return m_children;
  }

  bool                        isAncestorOf(std::shared_ptr<LayoutNode> node);
  std::shared_ptr<LayoutNode> closestCommonAncestor(std::shared_ptr<LayoutNode> node);

  Layout::Rect frame() const;
  Layout::Rect bounds() const;
  void         setFrame(
            const Layout::Rect& newFrame,
            bool                updateRule = false,
            bool                useOldFrame = false,
            bool                duringLayout = false);

  void dump(std::string indent = {});

  uint32_t backgroundColor();

public:
  std::shared_ptr<Layout::Internal::AutoLayout> autoLayout() const;
  std::shared_ptr<Layout::Internal::AutoLayout> createAutoLayout();
  void                                          configureAutoLayout();

  std::shared_ptr<LayoutNode>                   autoLayoutContainer();
  std::shared_ptr<Layout::Internal::AutoLayout> containerAutoLayout();

  void setNeedLayout();
  bool needsLayout() const
  {
    return m_needsLayout;
  }
  void layoutIfNeeded();

  std::shared_ptr<LayoutNode> scaleTo(
    const Layout::Size& newSize,
    bool                updateRule,
    bool                preservingOrigin); // return node that needs layout

  std::shared_ptr<LayoutNode> findDescendantNodeById(const std::string& id);

  Layout::Rect calculateResizedFrame(const Layout::Size& newSize);
  Layout::Rect frameToAncestor(std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    return convertRectToAncestor(frame(), ancestorNode);
  }
  Layout::Size rotatedSize(const Layout::Size& size);
  bool         shouldSwapWidthAndHeight();
  Layout::Size swapWidthAndHeightIfNeeded(Layout::Size size);

public:
  std::string id() const;
  std::string originalId() const;

  std::string vggId() const;
  std::string name() const;
  std::string type() const;

  bool isVisible() const;

private:
  bool isResizingAroundCenter() const;
  bool isVectorNetwork() const;
  bool isVectorNetworkDescendant() const;

  bool pointInside(Layout::Point point)
  {
    auto rect = convertRectToAncestor(frame());
    return rect.contains(point);
  }

  Layout::Rect convertRectToAncestor(
    Layout::Rect                rect,
    std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    return { converPointToAncestor(rect.origin, ancestorNode), rect.size };
  }

  Layout::Point converPointToAncestor(
    Layout::Point               point,
    std::shared_ptr<LayoutNode> ancestorNode = nullptr);

  void resizeChildNodes(
    const Layout::Size& oldContainerSize,
    const Layout::Size& newContainerSize,
    bool                onlyWhichHasAbsolutePostion = false);

  Layout::Point origin() const;
  Layout::Size  size() const;
  Layout::Rect  transformedFrame() const;
  void          saveOldFrame();
  void          saveChildrendOldFrame();

  void           updateModel(const Layout::Rect& frame);
  Layout::Point  modelOrigin() const;
  Layout::Rect   modelBounds() const;
  Layout::Matrix modelMatrix() const;
  Layout::Matrix modelMatrixWithoutTranslate() const;

  Layout::Rect resize(
    const Layout::Size&  oldContainerSize,
    const Layout::Size&  newContainerSize,
    const Layout::Point* parentOrigin = nullptr,
    bool                 dry = false);
  Layout::Rect resizeGroup(
    const Layout::Size&  oldContainerSize,
    const Layout::Size&  newContainerSize,
    const Layout::Point* parentOrigin);
  Layout::Rect resizeContour(
    Domain::ContourElement& contourElement,
    const Layout::Size&     oldContainerSize,
    const Layout::Size&     newContainerSize,
    const Layout::Point*    parentOrigin);
  void scaleContour(
    Domain::ContourElement& contourElement,
    const Layout::Rect&     oldFrame,
    const Layout::Rect&     newFrame);

  std::pair<Layout::Scalar, Layout::Scalar> resizeH(
    const Layout::Size&  oldContainerSize,
    const Layout::Size&  newContainerSize,
    Layout::Rect         oldFrame,
    const Layout::Point* parentOrigin) const; // return x, w
  std::pair<Layout::Scalar, Layout::Scalar> resizeV(
    const Layout::Size&  oldContainerSize,
    const Layout::Size&  newContainerSize,
    Layout::Rect         oldFrame,
    const Layout::Point* parentOrigin) const; // return y, h

  EResizing              horizontalResizing() const;
  EResizing              verticalResizing() const;
  EAdjustContentOnResize adjustContentOnResize() const;

  bool shouldSkip();
  bool isBooleanGroup();

  void updatePathNodeModel(
    const Layout::Rect&                     newFrame,
    const Layout::Matrix&                   matrix,
    const std::vector<Layout::BezierPoint>& newPoints);

  void saveOldRatio(
    const Layout::Size&  oldContainerSize,
    Layout::Rect         oldFrame,
    const Layout::Point* parentOrigin);
};

class StateTree : public LayoutNode
{
  std::weak_ptr<LayoutNode> m_pageNode;
  std::weak_ptr<LayoutNode> m_srcNode;
  std::string               m_srcNodeId;

  std::shared_ptr<Domain::StateTreeElement> m_treeElement;

public:
  StateTree(std::shared_ptr<LayoutNode> pageNode)
    : LayoutNode(std::weak_ptr<Domain::Element>{})
    , m_pageNode{ pageNode }
  {
  }

public:
  const LayoutNode* asPage() const override
  {
    return m_pageNode.lock().get();
  }

  const std::shared_ptr<LayoutNode> parent() const override
  {
    return m_srcNode.lock()->parent();
  }

  std::shared_ptr<VGG::Domain::Element> elementNode() const override
  {
    return m_srcNode.lock()->elementNode();
  }

public:
  std::shared_ptr<LayoutNode> srcNode() const;

  void setSrcNode(std::shared_ptr<LayoutNode> srcNode);

  void setTreeElement(std::shared_ptr<Domain::StateTreeElement> treeElement)
  {
    m_treeElement = treeElement;
  }
};

} // namespace VGG
