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

#include "Domain/Model/Element.hpp"

#include "Layout/BezierPoint.hpp"
#include "Layout/Helper.hpp"

#include "Utility/Log.hpp"

#include <algorithm>

using namespace VGG;
using namespace VGG::Domain;
using namespace VGG::Model;

namespace
{
constexpr auto K_SEPARATOR = "__";
constexpr auto K_BORDER_PREFIX = "style.borders";

struct ElementFactory
{
  std::shared_ptr<Element> operator()(const std::monostate& arg) const
  {
    return nullptr;
  }
  std::shared_ptr<Element> operator()(const Frame& frame) const
  {
    auto p = std::make_shared<FrameElement>(frame);
    p->buildSubtree();
    return p;
  }
  std::shared_ptr<Element> operator()(const Group& group) const
  {
    auto p = std::make_shared<GroupElement>(group);
    p->buildSubtree();
    return p;
  }
  std::shared_ptr<Element> operator()(const SymbolMaster& master) const
  {
    auto p = std::make_shared<SymbolMasterElement>(master);
    p->buildSubtree();
    return p;
  }
  std::shared_ptr<Element> operator()(const SymbolInstance& instance) const
  {
    return std::make_shared<SymbolInstanceElement>(instance);
  }

  std::shared_ptr<Element> operator()(const Text& text) const
  {
    return std::make_shared<TextElement>(text);
  }
  std::shared_ptr<Element> operator()(const Image& image) const
  {
    return std::make_shared<ImageElement>(image);
  }
  std::shared_ptr<Element> operator()(const Path& path) const
  {
    auto p = std::make_shared<PathElement>(path);
    p->buildSubtree();
    return p;
  }

  std::shared_ptr<Element> operator()(const Contour& contour) const
  {
    return std::make_shared<ContourElement>(contour);
  }
  std::shared_ptr<Element> operator()(const Ellipse& ellipse) const
  {
    return std::make_shared<EllipseElement>(ellipse);
  }
  std::shared_ptr<Element> operator()(const Polygon& polygon) const
  {
    return std::make_shared<PolygonElement>(polygon);
  }
  std::shared_ptr<Element> operator()(const Rectangle& rectangle) const
  {
    return std::make_shared<RectangleElement>(rectangle);
  }
  std::shared_ptr<Element> operator()(const Star& star) const
  {
    return std::make_shared<StarElement>(star);
  }
  std::shared_ptr<Element> operator()(const VectorNetwork& network) const
  {
    return std::make_shared<VectorNetworkElement>(network);
  }
};

std::string join(
  const std::vector<std::string>& instanceIdStack,
  const std::string&              separator = K_SEPARATOR)
{
  if (instanceIdStack.empty())
  {
    return {};
  }

  return std::accumulate(
    std::next(instanceIdStack.begin()),
    instanceIdStack.end(),
    instanceIdStack[0], // start with first element
    [&](const std::string& a, const std::string& b) { return a + separator + b; });
}

std::vector<std::string> split(const std::string& s, const std::string& delimiter = K_SEPARATOR)
{
  size_t                   pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string              token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
  {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}
} // namespace

// DesignDocument
DesignDocument::DesignDocument(const Model::DesignModel& designModel)
{
  m_designModel = std::make_shared<Model::DesignModel>(designModel);
}
void DesignDocument::buildSubtree()
{
  for (auto& frame : m_designModel->frames)
  {
    auto element = std::make_shared<FrameElement>(frame);
    element->buildSubtree();
    addChild(element);
  }
  m_designModel->frames.clear();
}

Model::DesignModel DesignDocument::treeModel() const
{
  auto retModel = *m_designModel;
  for (auto& child : children())
  {
    auto frameElement = std::dynamic_pointer_cast<FrameElement>(child);
    if (frameElement)
    {
      retModel.frames.push_back(frameElement->treeModel());
    }
  }

  return retModel;
}

// Element
void Element::setVisible(bool visible)
{
  auto model = this->object();
  if (!model)
  {
    return;
  }
  model->visible = visible;
}

std::string Element::type() const
{
  if (auto pObject = object())
  {
    nlohmann::json j = pObject->class_;
    return j;
  }
  return {};
}

nlohmann::json Element::jsonModel()
{
  return {};
}
void Element::updateJsonModel(const nlohmann::json& newJsonModel)
{
}
void Element::getToModel(Model::SubGeometryType& subGeometry)
{
}
void Element::updateModel(const Model::SubGeometryType& subGeometry)
{
}
void Element::updateBounds(double w, double h)
{
  if (auto model = object())
  {
    model->bounds.width = w;
    model->bounds.height = h;
  }
}
void Element::updateMatrix(double tx, double ty)
{
  if (auto model = object())
  {
    ASSERT(model->matrix.size() == 6);
    model->matrix[4] = tx;
    model->matrix[5] = ty;
  }
}

void Element::updateMatrix(const std::vector<double>& matrix)
{
  if (auto model = object())
  {
    ASSERT(model->matrix.size() == matrix.size());
    model->matrix = matrix;
  }
}

void Element::addChildren(const std::vector<ContainerChildType>& children)
{
  for (auto& child : children)
  {
    if (auto element = std::visit(ElementFactory{}, child))
    {
      addChild(element);
    }
  }
}

void Element::addKeyPrefix(const std::string& prefix)
{
  auto model = this->object();
  if (!model)
  {
    return;
  }

  model->id = prefix + model->id;
  if (model->overrideKey)
  {
    model->overrideKey = prefix + model->overrideKey.value();
  }
}

void Element::makeMaskIdUnique(Domain::SymbolInstanceElement& instance, const std::string& idPrefix)
{
  auto model = object();
  if (!model)
  {
    return;
  }

  for (auto& item : model->alphaMaskBy)
  {
    auto uniqueId = idPrefix + item.id;
    if (instance.findElementByKey({ uniqueId }, nullptr))
    {
      item.id = uniqueId;
    }
  }

  for (auto& item : model->outlineMaskBy)
  {
    auto uniqueId = idPrefix + item;
    if (instance.findElementByKey({ uniqueId }, nullptr))
    {
      item = uniqueId;
    }
  }

  for (auto& child : children())
  {
    child->makeMaskIdUnique(instance, idPrefix);
  }
}

void Element::update(const Model::ReferencedStyle& refStyle)
{
  auto model = object();
  if (!model)
  {
    return;
  }

  model->style = refStyle.style;
  if (refStyle.contextSettings)
  {
    model->contextSettings = refStyle.contextSettings.value();
  }
}

void Element::applyOverride(
  const std::string&        name,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds,
  bool                      recursively)
{
  if (object() == nullptr) // override object only, skip subshapes like contour
  {
    return;
  }

  nlohmann::json contentJson = jsonModel();
  applyOverrides(contentJson, name, value, outDirtyNodeIds);
  updateJsonModel(contentJson);

  if (recursively)
  {
    for (auto& child : children())
    {
      child->applyOverride(name, value, outDirtyNodeIds, recursively);
    }
  }
}

void Element::addSubGeometry(const SubGeometryType& subGeometry)
{
  if (auto element = std::visit(ElementFactory{}, subGeometry))
  {
    addChild(element);
  }
}

void Element::applyOverrides(
  nlohmann::json&           json,
  std::string               name,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds)
{
  // make name to json pointer string: x.y -> /x/y
  while (true)
  {
    auto index = name.find(".");
    if (index == std::string::npos)
    {
      break;
    }
    name[index] = '/';
  }

  nlohmann::json::json_pointer path{ "/" + name };
  std::stack<std::string>      reversedPath;
  while (!path.empty())
  {
    reversedPath.push(path.back());
    path.pop_back();
  }

  Layout::applyOverridesDetail(json, reversedPath, value, outDirtyNodeIds);
}

bool Element::isAncestorOf(const std::shared_ptr<Element>& element) const
{
  if (!element)
  {
    return false;
  }

  if (this == element.get())
  {
    return true;
  }

  for (auto& child : children())
  {
    if (child->isAncestorOf(element))
    {
      return true;
    }
  }

  return false;
}

std::shared_ptr<Element> Element::findElementByKey(
  const std::vector<std::string>& keyStack,
  std::vector<std::string>*       outInstanceIdStack)
{
  if (keyStack.empty())
  {
    return nullptr;
  }

  auto model = object();
  if (!model)
  {
    return nullptr;
  }

  std::vector<std::string>  tmpInstanceIdStack;
  std::vector<std::string>* theOutInstanceIdStack;
  if (outInstanceIdStack)
  {
    tmpInstanceIdStack = *outInstanceIdStack;
    theOutInstanceIdStack = outInstanceIdStack;
  }
  else
  {
    theOutInstanceIdStack = &tmpInstanceIdStack;
  }
  tmpInstanceIdStack.push_back(keyStack[0]);

  // 1. find by overrideKey first; 2. find by id
  const auto& firstObjectId = join(tmpInstanceIdStack);
  if (
    (model->overrideKey && (model->overrideKey.value() == firstObjectId)) ||
    model->id == firstObjectId)
  {
    auto target = shared_from_this();
    if (keyStack.size() == 1) // is last key
    {
      return target;
    }
    else
    {
      // the id is already prefixed: xxx__yyy__zzz;
      const auto originalId = split(target->object()->id).back();
      theOutInstanceIdStack->push_back(originalId);

      return target->findElementByKey(
        { keyStack.begin() + 1, keyStack.end() },
        theOutInstanceIdStack);
    }
  }

  for (auto& child : children())
  {
    if (auto found = child->findElementByKey(keyStack, outInstanceIdStack))
    {
      return found;
    }
  }

  return nullptr;
}

Layout::Rect Element::bounds() const
{
  if (auto pModel = model())
  {
    auto r = pModel->bounds;
    return { { r.x, r.y }, { r.width, r.height } };
  }

  return {};
}

Layout::Matrix Element::matrix() const
{
  if (auto pModel = model())
  {
    auto m = pModel->matrix;
    ASSERT(m.size() == 6);
    return { m[0], m[1], m[2], m[3], m[4], m[5] };
  }

  return {};
}

void Element::getToModel(Model::ContainerChildType& variantModel)
{
}
void Element::getTreeToModel(Model::SubGeometryType& subGeometry)
{
  getToModel(subGeometry);
}
void Element::getTreeToModel(Model::ContainerChildType& variantModel)
{
  getToModel(variantModel);
}

// FrameElement
FrameElement::FrameElement(const Model::Frame& frame)
{
  m_frame = std::make_shared<Model::Frame>(frame);
}
Frame* FrameElement::object() const
{
  return m_frame.get();
}
void FrameElement::buildSubtree()
{
  addChildren(m_frame->childObjects);
  m_frame->childObjects.clear();
}
nlohmann::json FrameElement::jsonModel()
{
  ASSERT(m_frame);
  return m_frame;
}
void FrameElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_frame);
  *m_frame = newJsonModel;
}
void FrameElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_frame);
  subGeometry = *m_frame;
}
Model::Frame FrameElement::treeModel() const
{
  auto retModel = *m_frame;
  for (auto& child : children())
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel);
    retModel.childObjects.push_back(variantModel);
  }
  return retModel;
}
void FrameElement::getTreeToModel(Model::SubGeometryType& subGeometry)
{
  subGeometry = treeModel();
}
void FrameElement::getTreeToModel(Model::ContainerChildType& variantModel)
{
  variantModel = treeModel();
}

// GroupElement
GroupElement::GroupElement(const Model::Group& group)
{
  m_group = std::make_shared<Model::Group>(group);
}
Group* GroupElement::object() const
{
  return m_group.get();
}
void GroupElement::buildSubtree()
{
  addChildren(m_group->childObjects);
  m_group->childObjects.clear();
}
nlohmann::json GroupElement::jsonModel()
{
  ASSERT(m_group);
  return m_group;
}
void GroupElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_group);
  *m_group = newJsonModel;
}
void GroupElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_group);
  subGeometry = *m_group;
}
Model::Group GroupElement::treeModel() const
{
  auto retModel = *m_group;
  for (auto& child : children())
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel);
    retModel.childObjects.push_back(variantModel);
  }
  return retModel;
}
void GroupElement::getTreeToModel(Model::SubGeometryType& subGeometry)
{
  subGeometry = treeModel();
}
void GroupElement::getTreeToModel(Model::ContainerChildType& variantModel)
{
  variantModel = treeModel();
}

void GroupElement::applyOverride(
  const std::string&        name,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds,
  bool                      recursively)
{
  recursively = false;

  const auto isBorder = name.rfind(K_BORDER_PREFIX, 0) == 0;
  if (isBorder && m_group->isVectorNetwork && m_group->isVectorNetwork.value())
  {
    recursively = true;
  }

  Element::applyOverride(name, value, outDirtyNodeIds, recursively);
}

// SymbolMasterElement
SymbolMasterElement::SymbolMasterElement(const Model::SymbolMaster& master)
{
  m_master = std::make_shared<Model::SymbolMaster>(master);
}
SymbolMaster* SymbolMasterElement::object() const
{
  return m_master.get();
}
void SymbolMasterElement::buildSubtree()
{
  addChildren(m_master->childObjects);
  m_master->childObjects.clear();
}
nlohmann::json SymbolMasterElement::jsonModel()
{
  ASSERT(m_master);
  return m_master;
}
void SymbolMasterElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_master);
  *m_master = newJsonModel;
}
void SymbolMasterElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_master);
  subGeometry = *m_master;
}
Model::SymbolMaster SymbolMasterElement::treeModel() const
{
  auto retModel = *m_master;
  for (auto& child : children())
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel);
    retModel.childObjects.push_back(variantModel);
  }
  return retModel;
}
void SymbolMasterElement::getTreeToModel(Model::SubGeometryType& subGeometry)
{
  subGeometry = treeModel();
}
void SymbolMasterElement::getTreeToModel(Model::ContainerChildType& variantModel)
{
  variantModel = treeModel();
}

// SymbolInstanceElement
SymbolInstanceElement::SymbolInstanceElement(const Model::SymbolInstance& instance)
{
  m_instance = std::make_shared<Model::SymbolInstance>(instance);
}
SymbolInstance* SymbolInstanceElement::object() const
{
  return m_instance.get();
}
std::string SymbolInstanceElement::masterId() const
{
  return m_instance->masterId;
}
std::string SymbolInstanceElement::masterOverrideKey() const
{
  if (m_master && m_master->overrideKey)
  {
    return m_master->overrideKey.value();
  }
  return {};
}
void SymbolInstanceElement::setMaster(const Model::SymbolMaster& master)
{
  ASSERT(master.id == masterId());
  if (master.id == masterId())
  {
    m_master = std::make_unique<Model::SymbolMaster>(master);
    clearChildren();
    addChildren(m_master->childObjects);
  }
}
void SymbolInstanceElement::updateMasterId(const std::string& masterId)
{
  clearChildren();
  m_instance->masterId = masterId;
}
void SymbolInstanceElement::updateVariableAssignments(const nlohmann::json& json)
{
  auto model = object();
  if (!model)
  {
    return;
  }

  std::vector<VariableAssign> newAssignments = json;
  model->variableAssignments = newAssignments;
}
void SymbolInstanceElement::updateBounds(const VGG::Layout::Rect& bounds)
{
  auto model = object();
  if (!model)
  {
    return;
  }

  model->bounds.x = bounds.origin.x;
  model->bounds.y = bounds.origin.y;
  model->bounds.width = bounds.size.width;
  model->bounds.height = bounds.size.height;
}
nlohmann::json SymbolInstanceElement::jsonModel()
{
  ASSERT(m_instance);
  return m_instance;
}
void SymbolInstanceElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_instance);
  *m_instance = newJsonModel;
}
void SymbolInstanceElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_instance);
  subGeometry = *m_instance;
}
Model::SymbolMaster SymbolInstanceElement::treeModel() const
{
  ASSERT(m_master);
  Model::SymbolMaster retModel;
  static_cast<Model::Object&>(retModel) = *m_instance;
  for (auto& child : children())
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel);
    retModel.childObjects.push_back(variantModel);
  }
  retModel.class_ = m_master->class_;
  retModel.radius = m_master->radius;
  return retModel;
}
void SymbolInstanceElement::getTreeToModel(Model::SubGeometryType& subGeometry)
{
  if (m_master)
  {
    subGeometry = treeModel();
  }
  else
  {
    subGeometry = *m_instance;
  }
}
void SymbolInstanceElement::getTreeToModel(Model::ContainerChildType& variantModel)
{
  if (m_master)
  {
    variantModel = treeModel();
  }
  else
  {
    variantModel = *m_instance;
  }
}
// TextElement
TextElement::TextElement(const Model::Text& text)
{
  m_text = std::make_shared<Model::Text>(text);
}
Text* TextElement::object() const
{
  return m_text.get();
}
void TextElement::updateFields(const nlohmann::json& json)
{
  nlohmann::json jsonModel = m_text;
  for (auto& el : json.items())
  {
    jsonModel[el.key()] = el.value();
  }
  Model::Text text = jsonModel;
  m_text = std::make_shared<Model::Text>(text);
}
void TextElement::update(const Model::ReferencedStyle& refStyle)
{
  Element::update(refStyle);
  if (refStyle.fontAttr)
  {
    m_text->fontAttr.clear();
    m_text->fontAttr.push_back(refStyle.fontAttr.value());
  }
}
nlohmann::json TextElement::jsonModel()
{
  ASSERT(m_text);
  return m_text;
}
void TextElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_text);
  *m_text = newJsonModel;
}
void TextElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_text);
  subGeometry = *m_text;
}
void TextElement::getToModel(Model::ContainerChildType& variantModel)
{
  ASSERT(m_text);
  variantModel = *m_text;
}

// ImageElement
ImageElement::ImageElement(const Model::Image& image)
{
  m_image = std::make_shared<Model::Image>(image);
}
Image* ImageElement::object() const
{
  return m_image.get();
}
nlohmann::json ImageElement::jsonModel()
{
  ASSERT(m_image);
  return m_image;
}
void ImageElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_image);
  *m_image = newJsonModel;
}
void ImageElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_image);
  subGeometry = *m_image;
}
void ImageElement::getToModel(Model::ContainerChildType& variantModel)
{
  ASSERT(m_image);
  variantModel = *m_image;
}

// PathElement
PathElement::PathElement(const Model::Path& path)
{
  m_path = std::make_shared<Model::Path>(path);
}
Model::Path* PathElement::object() const
{
  return m_path.get();
}
void PathElement::buildSubtree()
{
  if (m_path->shape)
  {
    for (auto& subshape : m_path->shape->subshapes)
    {
      if (subshape.subGeometry)
      {
        addSubGeometry(*subshape.subGeometry);
        subshape.subGeometry = nullptr;
      }
    }
  }
}
nlohmann::json PathElement::jsonModel()
{
  ASSERT(m_path);
  if (m_path->shape)
  {
    ASSERT(children().size() >= m_path->shape->subshapes.size());
    for (std::size_t i = 0; i < m_path->shape->subshapes.size(); i++)
    {
      auto subGeometry = std::make_shared<Model::SubGeometryType>();
      children()[i]->getToModel(*subGeometry);
      m_path->shape->subshapes[i].subGeometry = subGeometry;
    }
  }
  return m_path;
}
void PathElement::updateJsonModel(const nlohmann::json& newJsonModel)
{
  ASSERT(m_path);
  *m_path = newJsonModel;

  if (m_path->shape)
  {
    for (std::size_t i = 0; i < m_path->shape->subshapes.size(); i++)
    {
      auto& objectModel = *m_path->shape->subshapes[i].subGeometry;
      children()[i]->updateModel(objectModel);
      m_path->shape->subshapes[i].subGeometry = nullptr;
    }
  }
}
void PathElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_path);
  subGeometry = *m_path;
}
Model::Path PathElement::treeModel() const
{
  auto retModel = *m_path;
  if (m_path->shape)
  {
    ASSERT(children().size() >= m_path->shape->subshapes.size());
    for (std::size_t i = 0; i < m_path->shape->subshapes.size(); i++)
    {
      auto variantModel = std::make_shared<SubGeometryType>();
      children()[i]->getTreeToModel(*variantModel);
      retModel.shape->subshapes[i].subGeometry = variantModel;
    }
  }
  return retModel;
}
void PathElement::getTreeToModel(Model::SubGeometryType& subGeometry)
{
  subGeometry = treeModel();
}
void PathElement::getTreeToModel(Model::ContainerChildType& variantModel)
{
  variantModel = treeModel();
}

// ContourElement
ContourElement::ContourElement(const Model::Contour& contour)
{
  m_contour = std::make_shared<Model::Contour>(contour);
}

std::vector<Layout::BezierPoint> ContourElement::points() const
{
  ASSERT(m_contour);

  std::vector<Layout::BezierPoint> result;
  for (auto& point : m_contour->points)
  {
    result.push_back(Layout::BezierPoint::makeFromModel(point));
  }

  return result;
}

Model::Contour* ContourElement::dataModel() const
{
  ASSERT(m_contour);
  return m_contour.get();
}

void ContourElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_contour);
  subGeometry = *m_contour;
}
void ContourElement::updateModel(const Model::SubGeometryType& subGeometry)
{
  ASSERT(m_contour);
  if (auto p = std::get_if<Model::Contour>(&subGeometry))
  {
    *m_contour = *p;
  }
}
void ContourElement::updatePoints(const std::vector<Layout::BezierPoint>& points)
{
  if (!m_contour)
  {
    return;
  }
  ASSERT(m_contour->points.size() == points.size());
  for (std::size_t i = 0; i < m_contour->points.size(); i++)
  {
    m_contour->points[i].point[0] = points[i].point.x;
    m_contour->points[i].point[1] = points[i].point.y;
    if (points[i].from)
    {
      (*m_contour->points[i].curveFrom)[0] = points[i].from->x;
      (*m_contour->points[i].curveFrom)[1] = points[i].from->y;
    }
    if (points[i].to)
    {
      (*m_contour->points[i].curveTo)[0] = points[i].to->x;
      (*m_contour->points[i].curveTo)[1] = points[i].to->y;
    }
  }
}

// EllipseElement
EllipseElement::EllipseElement(const Model::Ellipse& ellipse)
{
  m_ellipse = std::make_shared<Model::Ellipse>(ellipse);
}
void EllipseElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_ellipse);
  subGeometry = *m_ellipse;
}
void EllipseElement::updateModel(const Model::SubGeometryType& subGeometry)
{
  ASSERT(m_ellipse);
  if (auto p = std::get_if<Model::Ellipse>(&subGeometry))
  {
    *m_ellipse = *p;
  }
}

// PolygonElement
PolygonElement::PolygonElement(const Model::Polygon& polygon)
{
  m_polygon = std::make_shared<Model::Polygon>(polygon);
}
void PolygonElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_polygon);
  subGeometry = *m_polygon;
}
void PolygonElement::updateModel(const Model::SubGeometryType& subGeometry)
{
  ASSERT(m_polygon);
  if (auto p = std::get_if<Model::Polygon>(&subGeometry))
  {
    *m_polygon = *p;
  }
}

// RectangleElement
RectangleElement::RectangleElement(const Model::Rectangle& rectangle)
{
  m_rectangle = std::make_shared<Model::Rectangle>(rectangle);
}
void RectangleElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_rectangle);
  subGeometry = *m_rectangle;
}
void RectangleElement::updateModel(const Model::SubGeometryType& subGeometry)
{
  ASSERT(m_rectangle);
  if (auto p = std::get_if<Model::Rectangle>(&subGeometry))
  {
    *m_rectangle = *p;
  }
}

// StarElement
StarElement::StarElement(const Model::Star& star)
{
  m_star = std::make_shared<Model::Star>(star);
}
void StarElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_star);
  subGeometry = *m_star;
}
void StarElement::updateModel(const Model::SubGeometryType& subGeometry)
{
  ASSERT(m_star);
  if (auto p = std::get_if<Model::Star>(&subGeometry))
  {
    *m_star = *p;
  }
}

// VectorNetworkElement
VectorNetworkElement::VectorNetworkElement(const Model::VectorNetwork& network)
{
  m_vectorNetwork = std::make_shared<Model::VectorNetwork>(network);
}
void VectorNetworkElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_vectorNetwork);
  subGeometry = *m_vectorNetwork;
}
void VectorNetworkElement::updateModel(const Model::SubGeometryType& subGeometry)
{
  ASSERT(m_vectorNetwork);
  if (auto p = std::get_if<Model::VectorNetwork>(&subGeometry))
  {
    *m_vectorNetwork = *p;
  }
}
