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

#include "Domain/Layout/Rect.hpp"
#include "Domain/Model/DesignModel.hpp"

#include <memory>
#include <vector>

namespace VGG
{

namespace Model
{
struct DesignModel;
struct Object;
struct SymbolInstance;
struct SymbolMaster;
} // namespace Model

namespace Domain
{
class SymbolInstanceElement;

class Element : public std::enable_shared_from_this<Element>
{
  std::weak_ptr<Element>                m_parent;
  std::vector<std::shared_ptr<Element>> m_children;

public:
  virtual ~Element() = default;

public: // Getters, Queries
  const std::shared_ptr<Element> parent() const
  {
    return m_parent.lock();
  }

  const std::vector<std::shared_ptr<Element>>& children() const
  {
    return m_children;
  }

  bool isLayoutNode() const
  {
    return object();
  }
  std::string id() const
  {
    if (auto obj = object())
    {
      return obj->id;
    }
    return "";
  }
  virtual const Model::Object* model() const
  {
    return object();
  }

  bool isAncestorOf(const std::shared_ptr<Element>& element) const;

  std::shared_ptr<Element> findElementByKey(
    const std::vector<std::string>& keyStack,
    std::vector<std::string>*       outInstanceIdStack);

public: // Setters, Commands
  void setVisible(bool visible);

  void addChild(std::shared_ptr<Element> child)
  {
    if (!child)
    {
      return;
    }

    child->m_parent = weak_from_this();
    m_children.push_back(child);
  }

  void clearChildren()
  {
    m_children.clear();
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
    std::vector<std::string>& outDirtyNodeIds);

  virtual Model::Object* object() const
  {
    return nullptr;
  }
  virtual nlohmann::json jsonModel();
  virtual void           updateJsonModel(const nlohmann::json& newJsonModel);
  virtual void           getToModel(Model::SubGeometryType& subGeometry);
  virtual void           updateModel(const Model::SubGeometryType& subGeometry);

  void addChildren(const std::vector<Model::ContainerChildType>& children);
  void addSubGeometry(const Model::SubGeometryType& subGeometry);

private:
  void applyOverrides(
    nlohmann::json&           json,
    std::string               name,
    const nlohmann::json&     value,
    std::vector<std::string>& outDirtyNodeIds);
};

class DesignDocument : public Element
{
  std::shared_ptr<Model::DesignModel> m_designModel;

public:
  DesignDocument(const Model::DesignModel& designModel);

  void buildSubtree() override;
};

class FrameElement : public Element
{
  std::shared_ptr<Model::Frame> m_frame;

public:
  FrameElement(const Model::Frame& frame);

  void buildSubtree() override;

  Model::Object* object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
};

class GroupElement : public Element
{
  std::shared_ptr<Model::Group> m_group;

public:
  GroupElement(const Model::Group& group);

  void buildSubtree() override;

  Model::Object* object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
};

class SymbolMasterElement : public Element
{
  std::shared_ptr<Model::SymbolMaster> m_master;

public:
  SymbolMasterElement(const Model::SymbolMaster& master);

  void buildSubtree() override;

  Model::Object* object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
};

class SymbolInstanceElement : public Element
{
  std::shared_ptr<Model::SymbolInstance> m_instance;
  std::unique_ptr<Model::SymbolMaster>   m_master;

public:
  SymbolInstanceElement(const Model::SymbolInstance& instance);

  std::string masterId() const;
  std::string masterOverrideKey() const;

  virtual const Model::SymbolInstance* model() const override
  {
    return object();
  }

  void setMaster(const Model::SymbolMaster& master);
  void updateMasterId(const std::string& masterId);
  void updateVariableAssignments(const nlohmann::json& json);
  void updateBounds(const VGG::Layout::Rect& bounds);

  Model::SymbolInstance* object() const override;
  nlohmann::json         jsonModel() override;
  void                   updateJsonModel(const nlohmann::json& newJsonModel) override;
  void                   getToModel(Model::SubGeometryType& subGeometry) override;
};

class TextElement : public Element
{
  std::shared_ptr<Model::Text> m_text;

public:
  TextElement(const Model::Text& text);
  void updateFields(const nlohmann::json& json);

  void update(const Model::ReferencedStyle& refStyle) override;

  Model::Object* object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
};

class ImageElement : public Element
{
  std::shared_ptr<Model::Image> m_image;

public:
  ImageElement(const Model::Image& image);

  Model::Object* object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
};

class PathElement : public Element
{
  std::shared_ptr<Model::Path> m_path;

public:
  PathElement(const Model::Path& path);

  void buildSubtree() override;

  Model::Path*   object() const override;
  nlohmann::json jsonModel() override;
  void           updateJsonModel(const nlohmann::json& newJsonModel) override;
  void           getToModel(Model::SubGeometryType& subGeometry) override;
};

class ContourElement : public Element
{
  std::shared_ptr<Model::Contour> m_contour;

public:
  ContourElement(const Model::Contour& contour);

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class EllipseElement : public Element
{
  std::shared_ptr<Model::Ellipse> m_ellipse;

public:
  EllipseElement(const Model::Ellipse& ellipse);

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class PolygonElement : public Element
{
  std::shared_ptr<Model::Polygon> m_polygon;

public:
  PolygonElement(const Model::Polygon& polygon);

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class RectangleElement : public Element
{
  std::shared_ptr<Model::Rectangle> m_rectangle;

public:
  RectangleElement(const Model::Rectangle& rectangle);

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class StarElement : public Element
{
  std::shared_ptr<Model::Star> m_star;

public:
  StarElement(const Model::Star& star);

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

class VectorNetworkElement : public Element
{
  std::shared_ptr<Model::VectorNetwork> m_vectorNetwork;

public:
  VectorNetworkElement(const Model::VectorNetwork& vectorNetwork);

  void getToModel(Model::SubGeometryType& subGeometry) override;
  void updateModel(const Model::SubGeometryType& subGeometry) override;
};

} // namespace Domain
} // namespace VGG