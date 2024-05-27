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

#include <cstddef>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include "DesignModelFwd.hpp"
#include "Domain/Layout/Math.hpp"
#include "Domain/Layout/Rect.hpp"
#include "Domain/Model/DesignModel.hpp"
#include <nlohmann/json.hpp>
namespace VGG
{
namespace Domain
{
class SymbolInstanceElement;
}
namespace Layout
{
struct BezierPoint;
}
} // namespace VGG

namespace VGG
{

namespace Domain
{

class Element : public std::enable_shared_from_this<Element>
{
public:
  enum class EType
  {
    NONE,
    ROOT,
    FRAME,
    GROUP,
    IMAGE,
    PATH,
    SYMBOL_INSTANCE,
    SYMBOL_MASTER,
    TEXT,
    CONTOUR,
    ELLIPSE,
    POLYGON,
    RECTANGLE,
    STAR,
    VECTOR_NETWORK,
    STATE_TREE
  };

private:
  const EType m_type;
  int         m_idNumber;

  std::weak_ptr<Element>                m_parent;
  std::vector<std::shared_ptr<Element>> m_children;

  bool m_fistOnTop{ false };

public:
  Element(EType type)
    : m_type{ type }
    , m_idNumber{ generateId() }
  {
  }
  virtual ~Element() = default;

  std::shared_ptr<Element>         cloneTree() const;
  virtual std::shared_ptr<Element> clone() const = 0;
  std::size_t                      size() const;

  void regenerateId(bool recursively);

public:
  auto type() const
  {
    return m_type;
  }
  int idNumber() const
  {
    return m_idNumber;
  }

  const std::shared_ptr<Element> parent() const
  {
    return m_parent.lock();
  }

  std::vector<std::shared_ptr<Element>> children(bool reverseChildrenIfFirstOnTop = false) const;

  const auto& childObjects() const
  {
    return m_children;
  }

  auto begin() const noexcept
  {
    return m_children.begin();
  }

  auto end() const noexcept
  {
    return m_children.end();
  }

  auto cbegin() const noexcept
  {
    return m_children.cbegin();
  }

  auto cend() const noexcept
  {
    return m_children.cend();
  }

  bool isLayoutNode() const
  {
    return object();
  }
  const std::string& id() const;
  std::string        originalId() const;
  std::string        name() const;

  virtual const Model::Object* model() const
  {
    return object();
  }

  bool isAncestorOf(const std::shared_ptr<Element>& element) const;

  std::shared_ptr<Element> findElementByKey(
    const std::vector<std::string>& keyStack,
    std::vector<std::string>*       outInstanceIdStack);

  std::string    typeString() const;
  Layout::Rect   bounds() const;
  Layout::Matrix matrix() const;

  virtual std::shared_ptr<Element> getElementByKey(const std::string& key); // name or id

public: // Getters, Setters
  void setVisible(bool visible);

  bool isFirstOnTop() const
  {
    return m_fistOnTop;
  }
  void setFirstOnTop(bool firstOnTop)
  {
    m_fistOnTop = firstOnTop;
  }

public:
  void addChild(std::shared_ptr<Element> child)
  {
    if (!child)
    {
      return;
    }

    child->m_parent = weak_from_this();
    m_children.push_back(child);
  }

  auto clearChildren()
  {
    return std::move(m_children);
  }

  virtual void buildSubtree()
  {
  }

  void addKeyPrefix(const std::string& prefix);
  void makeMaskIdUnique(SymbolInstanceElement& instance, const std::string& idPrefix);

  virtual void update(const Model::ReferencedStyle& refStyle);
  virtual void applyOverride(
    const std::string&        name,
    const nlohmann::json&     value,
    std::vector<std::string>& outDirtyNodeIds,
    bool                      recursively = false);

  virtual Model::Object* object() const
  {
    return nullptr;
  }
  virtual nlohmann::json jsonModel();

  virtual void updateJsonModel(const nlohmann::json& newJsonModel);
  virtual void updateModel(const Model::SubGeometryType& subGeometry);
  void         updateBounds(double w, double h);
  void         updateMatrix(double tx, double ty);
  void         updateMatrix(const std::vector<double>& matrix);

  virtual void getToModel(Model::SubGeometryType& subGeometry);
  virtual void getToModel(Model::ContainerChildType& variantModel);
  virtual void getTreeToModel(
    Model::SubGeometryType& subGeometry,
    bool                    reverseChildrenIfFirstOnTop);
  virtual void getTreeToModel(
    Model::ContainerChildType& variantModel,
    bool                       reverseChildrenIfFirstOnTop);

  void addChildren(const std::vector<Model::ContainerChildType>& children);
  void addSubGeometry(const Model::SubGeometryType& subGeometry);

private:
  void applyOverrides(
    nlohmann::json&           json,
    std::string               name,
    const nlohmann::json&     value,
    std::vector<std::string>& outDirtyNodeIds);

  static int generateId();
};

class DesignDocument : public Element
{
  std::shared_ptr<Model::DesignModel> m_designModel;

public:
  DesignDocument(const Model::DesignModel& designModel);
  std::shared_ptr<Element> clone() const override;

  void               buildSubtree() override;
  Model::DesignModel treeModel(
    bool reverseChildrenIfFirstOnTop = false) const; // build the tree model

  std::shared_ptr<Element> getElementByKey(const std::string& key) override;

  std::shared_ptr<Model::DesignModel> designModel() const
  {
    return m_designModel;
  }
};

class FrameElement : public Element
{
  std::shared_ptr<Model::Frame> m_frame;

public:
  FrameElement(const Model::Frame& frame);
  std::shared_ptr<Element> clone() const override;

  void buildSubtree() override;

  Model::Frame*  object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  Model::Frame   treeModel(bool reverseChildrenIfFirstOnTop) const;

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void getTreeToModel(Model::SubGeometryType& subGeometry, bool reverseChildrenIfFirstOnTop)
    override;
  void getTreeToModel(Model::ContainerChildType& variantModel, bool reverseChildrenIfFirstOnTop)
    override;
};

class GroupElement : public Element
{
  std::shared_ptr<Model::Group> m_group;

public:
  GroupElement(const Model::Group& group);
  std::shared_ptr<Element> clone() const override;

  void buildSubtree() override;

  Model::Group*  object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
  Model::Group   treeModel(bool reverseChildrenIfFirstOnTop) const;
  void getTreeToModel(Model::SubGeometryType& subGeometry, bool reverseChildrenIfFirstOnTop)
    override;
  void getTreeToModel(Model::ContainerChildType& variantModel, bool reverseChildrenIfFirstOnTop)
    override;

  void applyOverride(
    const std::string&        name,
    const nlohmann::json&     value,
    std::vector<std::string>& outDirtyNodeIds,
    bool                      recursively) override;
};

class SymbolMasterElement : public Element
{
  std::shared_ptr<Model::SymbolMaster> m_master;

public:
  SymbolMasterElement(const Model::SymbolMaster& master);
  std::shared_ptr<Element> clone() const override;

  void buildSubtree() override;

  Model::SymbolMaster* object() const override;
  nlohmann::json       jsonModel() override;
  void                 updateJsonModel(const nlohmann::json& newJsonModel) override;
  void                 getToModel(Model::SubGeometryType& subGeometry) override;
  Model::SymbolMaster  treeModel(bool reverseChildrenIfFirstOnTop) const;
  void getTreeToModel(Model::SubGeometryType& subGeometry, bool reverseChildrenIfFirstOnTop)
    override;
  void getTreeToModel(Model::ContainerChildType& variantModel, bool reverseChildrenIfFirstOnTop)
    override;
};

class SymbolInstanceElement : public Element
{
  std::shared_ptr<Model::SymbolInstance> m_instance;
  std::unique_ptr<Model::SymbolMaster>   m_master;
  std::stack<std::string>                m_stateStack; // master id stack

public:
  SymbolInstanceElement(const Model::SymbolInstance& instance);
  std::shared_ptr<Element> clone() const override;

  std::string masterId() const;
  std::string masterOverrideKey() const;

  virtual const Model::SymbolInstance* model() const override
  {
    return object();
  }

  void                                  setMaster(const Model::SymbolMaster& master);
  std::vector<std::shared_ptr<Element>> updateMasterId(const std::string& masterId);
  void                                  updateVariableAssignments(const nlohmann::json& json);
  void                                  updateBounds(const VGG::Layout::Rect& bounds);

  std::vector<std::shared_ptr<Element>> presentState(const std::string& masterId);
  std::string                           dissmissState();
  void                                  resetState();

  Model::SymbolInstance* object() const override;
  nlohmann::json         jsonModel() override;
  void                   updateJsonModel(const nlohmann::json& newJsonModel) override;
  void                   getToModel(Model::SubGeometryType& subGeometry) override;
  Model::SymbolMaster    treeModel(bool reverseChildrenIfFirstOnTop) const;
  void getTreeToModel(Model::SubGeometryType& subGeometry, bool reverseChildrenIfFirstOnTop)
    override;
  void getTreeToModel(Model::ContainerChildType& variantModel, bool reverseChildrenIfFirstOnTop)
    override;
};

class TextElement : public Element
{
  std::shared_ptr<Model::Text> m_text;

public:
  TextElement(const Model::Text& text);
  std::shared_ptr<Element> clone() const override;

  void updateFields(const nlohmann::json& json);

  void update(const Model::ReferencedStyle& refStyle) override;

  Model::Text*   object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
  void           getToModel(Model::ContainerChildType& variantModel) override;
};

class ImageElement : public Element
{
  std::shared_ptr<Model::Image> m_image;

public:
  ImageElement(const Model::Image& image);
  std::shared_ptr<Element> clone() const override;

  Model::Image*  object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
  void           getToModel(Model::ContainerChildType& variantModel) override;
};

class PathElement : public Element
{
  std::shared_ptr<Model::Path> m_path;

public:
  PathElement(const Model::Path& path);
  std::shared_ptr<Element> clone() const override;

  void buildSubtree() override;

  Model::Path*   object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
  Model::Path    treeModel(bool reverseChildrenIfFirstOnTop) const;
  void getTreeToModel(Model::SubGeometryType& subGeometry, bool reverseChildrenIfFirstOnTop)
    override;
  void getTreeToModel(Model::ContainerChildType& variantModel, bool reverseChildrenIfFirstOnTop)
    override;
};

class ContourElement : public Element
{
  std::shared_ptr<Model::Contour> m_contour;

public:
  ContourElement(const Model::Contour& contour);
  std::shared_ptr<Element> clone() const override;

  std::vector<Layout::BezierPoint> points() const;

  Model::Contour* dataModel() const;

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
  void updatePoints(const std::vector<Layout::BezierPoint>& points);
};

class EllipseElement : public Element
{
  std::shared_ptr<Model::Ellipse> m_ellipse;
  std::shared_ptr<Element>        clone() const override;

public:
  EllipseElement(const Model::Ellipse& ellipse);

  Model::Ellipse* dataModel() const
  {
    return m_ellipse.get();
  }

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class PolygonElement : public Element
{
  std::shared_ptr<Model::Polygon> m_polygon;

public:
  PolygonElement(const Model::Polygon& polygon);
  std::shared_ptr<Element> clone() const override;

  Model::Polygon* dataModel() const
  {
    return m_polygon.get();
  }

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class RectangleElement : public Element
{
  std::shared_ptr<Model::Rectangle> m_rectangle;

public:
  RectangleElement(const Model::Rectangle& rectangle);
  std::shared_ptr<Element> clone() const override;

  Model::Rectangle* dataModel() const
  {
    return m_rectangle.get();
  }

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class StarElement : public Element
{
  std::shared_ptr<Model::Star> m_star;

public:
  StarElement(const Model::Star& star);
  std::shared_ptr<Element> clone() const override;

  Model::Star* dataModel() const
  {
    return m_star.get();
  }

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class VectorNetworkElement : public Element
{
  std::shared_ptr<Model::VectorNetwork> m_vectorNetwork;

public:
  VectorNetworkElement(const Model::VectorNetwork& vectorNetwork);
  std::shared_ptr<Element> clone() const override;

  Model::VectorNetwork* dataModel() const
  {
    return m_vectorNetwork.get();
  }

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class StateTreeElement : public Element
{
  std::weak_ptr<Element> m_srcElement;

public:
  StateTreeElement(std::shared_ptr<Element> srcElement);
  std::shared_ptr<Element> clone() const override;
};

} // namespace Domain
} // namespace VGG
