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

#include "Domain/Model/Element.hpp"
#include <algorithm>
#include <optional>
#include <variant>
#include "Domain/Model/DesignModel.hpp"
#include "Layout/BezierPoint.hpp"
#include "Layout/Helper.hpp"
#include "Math.hpp"
#include "Rect.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggString.hpp"
#include <nlohmann/json.hpp>

#undef DEBUG
#define DEBUG(msg, ...)
namespace VGG::Domain
{

namespace
{
using namespace Model;

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

} // namespace

// DesignDocument
DesignDocument::DesignDocument(const Model::DesignModel& designModel)
  : Element(EType::ROOT)
{
  m_designModel = std::make_shared<Model::DesignModel>(designModel);
}

std::shared_ptr<Element> DesignDocument::clone() const
{
  return std::make_shared<DesignDocument>(*m_designModel);
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

Model::DesignModel DesignDocument::treeModel(bool reverseChildrenIfFirstOnTop) const
{
  auto retModel = *m_designModel;
  for (auto& child : children(reverseChildrenIfFirstOnTop))
  {
    auto frameElement = std::dynamic_pointer_cast<FrameElement>(child);
    if (frameElement)
    {
      retModel.frames.push_back(frameElement->treeModel(reverseChildrenIfFirstOnTop));
    }
  }

  return retModel;
}

std::shared_ptr<Element> DesignDocument::getElementByKey(const std::string& key)
{
  // design document has no object, so we need to find in its children
  for (auto& child : children())
  {
    if (auto element = child->getElementByKey(key))
    {
      return element;
    }
  }

  return nullptr;
}

// Element
int Element::generateId()
{
  static int s_id{ 0 };
  return ++s_id;
}

std::shared_ptr<Element> Element::cloneTree() const
{
  auto n = clone();
  n->m_idNumber = idNumber(); // clone id number
  for (auto& child : childObjects())
    n->addChild(child->cloneTree());
  return n;
}

void Element::regenerateId(bool recursively)
{
  m_idNumber = generateId();
  if (recursively)
    for (auto& child : childObjects())
      child->regenerateId(recursively);
}

std::size_t Element::size() const
{
  std::size_t count = 1;
  for (const auto& child : children())
    count += child->size();
  return count;
}

std::vector<std::shared_ptr<Element>> Element::children(bool reverseChildrenIfFirstOnTop) const
{
  auto result = m_children;
  if (reverseChildrenIfFirstOnTop && isFirstOnTop())
  {
    std::reverse(result.begin(), result.end());
  }
  return result;
}

void Element::setVisible(bool visible)
{
  auto model = this->object();
  if (!model)
  {
    return;
  }
  model->visible = visible;
}

std::string Element::typeString() const
{
  if (auto pObject = object())
  {
    nlohmann::json j = pObject->class_;
    return j;
  }
  return {};
}

const std::string& Element::id() const
{
  if (auto obj = object())
  {
    return obj->id;
  }

  static const std::string emptyId;
  return emptyId;
}

std::string Element::originalId() const
{
  return Helper::split(id()).back();
}

std::string Element::name() const
{
  if (auto obj = object())
  {
    if (obj->name)
    {
      return obj->name.value();
    }
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
    DEBUG("Element::updateMatrix, [%s, %p], %f, %f", id().c_str(), this, tx, ty);
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

  DEBUG(
    "Element::applyOverride: [%s], name %s, value %s",
    id().c_str(),
    name.c_str(),
    value.dump().c_str());
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
  const auto& firstObjectId = Helper::join(tmpInstanceIdStack);
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
      const auto originalId = Helper::split(target->object()->id).back();
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

Element* Element::findElementByRef(
  const std::vector<Element*>& refTargetReversedPath,
  std::size_t                  index,
  std::vector<std::string>*    outInstanceIdStack)
{
  ASSERT(index < refTargetReversedPath.size());
  auto refAncestor = refTargetReversedPath[index];

  if (name() != refAncestor->name())
    return nullptr;

  if (index > 0)
  {
    for (auto& child : children())
      if (
        auto found = child->findElementByRef(refTargetReversedPath, index - 1, outInstanceIdStack))
      {
        if (outInstanceIdStack && (child->type() == EType::SYMBOL_INSTANCE))
          outInstanceIdStack->push_back(Helper::split(child->id()).back());

        return found;
      }

    return nullptr;
  }
  else
    return this;
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

std::shared_ptr<Element> Element::getElementByKey(const std::string& key)
{
  if (auto pModel = object())
  {
    if (pModel->id == key || pModel->name == key)
    {
      return shared_from_this();
    }

    for (auto& child : children())
    {
      if (auto element = child->getElementByKey(key))
      {
        return element;
      }
    }
  }

  return nullptr;
}

void Element::getToModel(Model::ContainerChildType& variantModel)
{
}
void Element::getTreeToModel(Model::SubGeometryType& subGeometry, bool reverseChildrenIfFirstOnTop)
{
  getToModel(subGeometry);
}
void Element::getTreeToModel(
  Model::ContainerChildType& variantModel,
  bool                       reverseChildrenIfFirstOnTop)
{
  getToModel(variantModel);
}

// FrameElement
FrameElement::FrameElement(const Model::Frame& frame)
  : Element(EType::FRAME)
  , m_frame(new Model::Frame(frame))
  , m_shouldDisplay((frame.isNormal() && frame.visible))
{
  DEBUG("FrameElement::FrameElement, [%s, %p]", frame.id.c_str(), this);
}
std::shared_ptr<Element> FrameElement::clone() const
{
  return std::shared_ptr<Element>(new FrameElement(*object()));
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
  DEBUG(
    "Element::updateJsonModel, [%s, %p], %f, %f",
    id().c_str(),
    this,
    m_frame->matrix[4],
    m_frame->matrix[5]);
}
void FrameElement::getToModel(Model::SubGeometryType& subGeometry)
{
  ASSERT(m_frame);
  subGeometry = *m_frame;
}
Model::Frame FrameElement::treeModel(bool reverseChildrenIfFirstOnTop) const
{
  auto retModel = *m_frame;
  for (auto& child : children(reverseChildrenIfFirstOnTop))
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel, reverseChildrenIfFirstOnTop);
    retModel.childObjects.push_back(variantModel);
  }
  return retModel;
}
void FrameElement::getTreeToModel(
  Model::SubGeometryType& subGeometry,
  bool                    reverseChildrenIfFirstOnTop)
{
  subGeometry = treeModel(reverseChildrenIfFirstOnTop);
}
void FrameElement::getTreeToModel(
  Model::ContainerChildType& variantModel,
  bool                       reverseChildrenIfFirstOnTop)
{
  variantModel = treeModel(reverseChildrenIfFirstOnTop);
}
bool FrameElement::shouldDisplay() const
{
  return true; // TODO: filter frame by type and visible
  // return m_shouldDisplay;
}

// GroupElement
GroupElement::GroupElement(const Model::Group& group)
  : Element(EType::GROUP)
{
  m_group = std::make_shared<Model::Group>(group);
}
std::shared_ptr<Element> GroupElement::clone() const
{
  return std::make_shared<GroupElement>(*object());
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
Model::Group GroupElement::treeModel(bool reverseChildrenIfFirstOnTop) const
{
  auto retModel = *m_group;
  for (auto& child : children(reverseChildrenIfFirstOnTop))
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel, reverseChildrenIfFirstOnTop);
    retModel.childObjects.push_back(variantModel);
  }
  return retModel;
}
void GroupElement::getTreeToModel(
  Model::SubGeometryType& subGeometry,
  bool                    reverseChildrenIfFirstOnTop)
{
  subGeometry = treeModel(reverseChildrenIfFirstOnTop);
}
void GroupElement::getTreeToModel(
  Model::ContainerChildType& variantModel,
  bool                       reverseChildrenIfFirstOnTop)
{
  variantModel = treeModel(reverseChildrenIfFirstOnTop);
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
  : Element(EType::SYMBOL_MASTER)
{
  m_master = std::make_shared<Model::SymbolMaster>(master);
}
std::shared_ptr<Element> SymbolMasterElement::clone() const
{
  return std::make_shared<SymbolMasterElement>(*object());
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
Model::SymbolMaster SymbolMasterElement::treeModel(bool reverseChildrenIfFirstOnTop) const
{
  auto retModel = *m_master;
  for (auto& child : children(reverseChildrenIfFirstOnTop))
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel, reverseChildrenIfFirstOnTop);
    retModel.childObjects.push_back(variantModel);
  }
  return retModel;
}
void SymbolMasterElement::getTreeToModel(
  Model::SubGeometryType& subGeometry,
  bool                    reverseChildrenIfFirstOnTop)
{
  subGeometry = treeModel(reverseChildrenIfFirstOnTop);
}
void SymbolMasterElement::getTreeToModel(
  Model::ContainerChildType& variantModel,
  bool                       reverseChildrenIfFirstOnTop)
{
  variantModel = treeModel(reverseChildrenIfFirstOnTop);
}

// SymbolInstanceElement
SymbolInstanceElement::SymbolInstanceElement(const Model::SymbolInstance& instance)
  : Element(EType::SYMBOL_INSTANCE)
{
  m_instance = std::make_shared<Model::SymbolInstance>(instance);
}
std::shared_ptr<Element> SymbolInstanceElement::clone() const
{
  return std::make_shared<SymbolInstanceElement>(*object());
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

  m_master = std::make_unique<Model::SymbolMaster>(master);
  clearChildren();
  addChildren(m_master->childObjects);

  object()->style = m_master->style;
  object()->variableDefs = m_master->variableDefs;
}

std::vector<std::shared_ptr<Element>> SymbolInstanceElement::updateMasterId(
  const std::string& masterId)
{
  m_instance->masterId = masterId;
  return clearChildren();
}

std::vector<std::shared_ptr<Element>> SymbolInstanceElement::presentState(
  const std::string& newStateMasterId)
{
  if (masterId() == newStateMasterId)
  {
    return {};
  }
  m_stateStack.push(masterId());
  return updateMasterId(newStateMasterId);
}

std::string SymbolInstanceElement::dissmissState()
{
  if (m_stateStack.empty())
  {
    return {};
  }

  auto lastStateMasterId = m_stateStack.top();
  m_stateStack.pop();
  return lastStateMasterId;
}

void SymbolInstanceElement::resetState()
{
  while (!m_stateStack.empty())
  {
    m_stateStack.pop();
  }
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
Model::SymbolMaster SymbolInstanceElement::treeModel(bool reverseChildrenIfFirstOnTop) const
{
  ASSERT(m_master);
  Model::SymbolMaster retModel;
  static_cast<Model::Object&>(retModel) = *m_instance;
  for (auto& child : children(reverseChildrenIfFirstOnTop))
  {
    ContainerChildType variantModel;
    child->getTreeToModel(variantModel, reverseChildrenIfFirstOnTop);
    retModel.childObjects.push_back(variantModel);
  }
  retModel.class_ = m_master->class_;
  retModel.radius = m_master->radius;
  return retModel;
}
void SymbolInstanceElement::getTreeToModel(
  Model::SubGeometryType& subGeometry,
  bool                    reverseChildrenIfFirstOnTop)
{
  if (m_master)
  {
    subGeometry = treeModel(reverseChildrenIfFirstOnTop);
  }
  else
  {
    subGeometry = *m_instance;
  }
}
void SymbolInstanceElement::getTreeToModel(
  Model::ContainerChildType& variantModel,
  bool                       reverseChildrenIfFirstOnTop)
{
  if (m_master)
  {
    variantModel = treeModel(reverseChildrenIfFirstOnTop);
  }
  else
  {
    variantModel = *m_instance;
  }
}

void SymbolInstanceElement::saveOverrideTreeIfNeeded()
{
  if (m_overrideReferenceTree)
    return;
  m_overrideReferenceTree = std::static_pointer_cast<SymbolInstanceElement>(cloneTree());
  if (m_master)
    m_overrideReferenceTree->m_master = std::make_unique<Model::SymbolMaster>(*m_master);
}

SymbolInstanceElement* SymbolInstanceElement::overrideReferenceTree()
{
  return m_overrideReferenceTree.get();
}

bool SymbolInstanceElement::shouldKeepListeners()
{
  if (!m_stateStack.empty()) // is presenting state
    return true;

  if (auto s = overrideReferenceTree())
    return masterId() == s->masterId();

  return true;
}

// TextElement
TextElement::TextElement(const Model::Text& text)
  : Element(EType::TEXT)
{
  m_text = std::make_shared<Model::Text>(text);
}
std::shared_ptr<Element> TextElement::clone() const
{
  return std::make_shared<TextElement>(*object());
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
  : Element(EType::IMAGE)
{
  m_image = std::make_shared<Model::Image>(image);
}
std::shared_ptr<Element> ImageElement::clone() const
{
  return std::make_shared<ImageElement>(*object());
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
  : Element(EType::PATH)
{
  m_path = std::make_shared<Model::Path>(path);
}
std::shared_ptr<Element> PathElement::clone() const
{
  return std::make_shared<PathElement>(*object());
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
Model::Path PathElement::treeModel(bool reverseChildrenIfFirstOnTop) const
{
  auto retModel = *m_path;
  if (m_path->shape)
  {
    ASSERT(children().size() >= m_path->shape->subshapes.size());
    for (std::size_t i = 0; i < m_path->shape->subshapes.size(); i++)
    {
      auto variantModel = std::make_shared<SubGeometryType>();
      children()[i]->getTreeToModel(*variantModel, reverseChildrenIfFirstOnTop);
      retModel.shape->subshapes[i].subGeometry = variantModel;
    }
    if (reverseChildrenIfFirstOnTop && isFirstOnTop())
    {
      std::reverse(retModel.shape->subshapes.begin(), retModel.shape->subshapes.end());
    }
  }
  return retModel;
}
void PathElement::getTreeToModel(
  Model::SubGeometryType& subGeometry,
  bool                    reverseChildrenIfFirstOnTop)
{
  subGeometry = treeModel(reverseChildrenIfFirstOnTop);
}
void PathElement::getTreeToModel(
  Model::ContainerChildType& variantModel,
  bool                       reverseChildrenIfFirstOnTop)
{
  variantModel = treeModel(reverseChildrenIfFirstOnTop);
}

// ContourElement
ContourElement::ContourElement(const Model::Contour& contour)
  : Element(EType::CONTOUR)
{
  m_contour = std::make_shared<Model::Contour>(contour);
}
std::shared_ptr<Element> ContourElement::clone() const
{
  return std::make_shared<ContourElement>(*dataModel());
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
  : Element(EType::ELLIPSE)
{
  m_ellipse = std::make_shared<Model::Ellipse>(ellipse);
}
std::shared_ptr<Element> EllipseElement::clone() const
{
  return std::make_shared<EllipseElement>(*m_ellipse);
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
  : Element(EType::POLYGON)
{
  m_polygon = std::make_shared<Model::Polygon>(polygon);
}
std::shared_ptr<Element> PolygonElement::clone() const
{
  return std::make_shared<PolygonElement>(*m_polygon);
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
  : Element(EType::RECTANGLE)
{
  m_rectangle = std::make_shared<Model::Rectangle>(rectangle);
}
std::shared_ptr<Element> RectangleElement::clone() const
{
  return std::make_shared<RectangleElement>(*m_rectangle);
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
  : Element(EType::STAR)
{
  m_star = std::make_shared<Model::Star>(star);
}
std::shared_ptr<Element> StarElement::clone() const
{
  return std::make_shared<StarElement>(*m_star);
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
  : Element(EType::VECTOR_NETWORK)
{
  m_vectorNetwork = std::make_shared<Model::VectorNetwork>(network);
}
std::shared_ptr<Element> VectorNetworkElement::clone() const
{
  return std::make_shared<VectorNetworkElement>(*m_vectorNetwork);
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

// StateTreeElement
StateTreeElement::StateTreeElement(std::shared_ptr<Element> srcElement)
  : Element(EType::STATE_TREE)
  , m_srcElement{ srcElement }
{
}
std::shared_ptr<Element> StateTreeElement::clone() const
{
  return {};
}

} // namespace VGG::Domain
