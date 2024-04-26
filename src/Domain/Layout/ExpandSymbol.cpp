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
#include "ExpandSymbol.hpp"

#include "Layout.hpp"
#include "ReferenceJsonDocument.hpp"
#include "Rule.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"

#include "Domain/Model/Element.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggString.hpp"

#include <algorithm>
#include <fstream>
#include <numeric>
#include <iostream>

#undef DEBUG
#define DEBUG(msg, ...)

// #define DUMP_TREE

constexpr auto K_PREFIX = "referenced_style_";

namespace nl = nlohmann;
using namespace VGG::Domain;
using namespace VGG::Layout;
using namespace VGG::Model;

namespace
{

enum class EVarType
{
  NONE,
  BOOLEAN,
  TEXT_PROPERTY,
  STRING,
  REFERENCE,
  MAX,
};

} // namespace

ExpandSymbol::ExpandSymbol(const nlohmann::json& designJson, const nlohmann::json& layoutJson)
  : m_designModel(new DesignModel(designJson))
  , m_layoutJson(layoutJson)
{
}

ExpandSymbol::~ExpandSymbol()
{
}

std::pair<std::shared_ptr<VGG::Domain::DesignDocument>, nlohmann::json> ExpandSymbol::operator()()
{
  collectMasters();
  collectLayoutRules(m_layoutJson);

  m_layoutRulesCache = Layout::collectRules(m_layoutJson);
  m_outLayoutJsonMap = m_layoutRules;

  m_designDocument = std::make_shared<Domain::DesignDocument>(*m_designModel);
  m_designDocument->buildSubtree();
  m_layout.reset(new Layout{ m_designDocument, getLayoutRules() });

  for (auto& page : m_designDocument->children())
  {
    std::vector<std::string> instanceIdStack{};
    traverseElementNode(page, instanceIdStack);
  }

  auto outLayoutJson = generateOutLayoutJson();

  return { m_designDocument, std::move(outLayoutJson) };
}

std::pair<nlohmann::json, nlohmann::json> ExpandSymbol::run()
{
  auto [designDocument, layoutJson] = operator()();
  return { designDocument->treeModel(), std::move(layoutJson) };
}

std::shared_ptr<VGG::Layout::Layout> ExpandSymbol::layout() const
{
  return m_layout;
}

void ExpandSymbol::collectMasters()
{
  for (auto& frame : m_designModel->frames)
  {
    collectMasterFromContainer(frame);
  }
  if (m_designModel->references)
  {
    auto& refs = *m_designModel->references;
    for (auto& ref : refs)
    {
      if (auto p = std::get_if<SymbolMaster>(&ref))
      {
        m_pMasters[p->id] = p;
      }
    }
  }
}

void ExpandSymbol::collectMasterFromContainer(const Model::Container& conntainer)
{
  for (auto& child : conntainer.childObjects)
  {
    collectMasterFromVariant(child);
  }
}

template<typename T>
void ExpandSymbol::collectMasterFromVariant(const T& variantNode)
{
  if (auto p = std::get_if<SymbolMaster>(&variantNode))
  {
    m_pMasters[p->id] = p;
    collectMasterFromContainer(*p);
  }
  else if (auto p = std::get_if<Frame>(&variantNode))
  {
    collectMasterFromContainer(*p);
  }
  else if (auto p = std::get_if<Group>(&variantNode))
  {
    collectMasterFromContainer(*p);
  }
  else if (auto p = std::get_if<Path>(&variantNode))
  {
    if (p->shape)
    {
      for (auto& subshape : p->shape->subshapes)
      {
        if (subshape.subGeometry)
        {
          collectMasterFromVariant(*subshape.subGeometry);
        }
      }
    }
  }
}

void ExpandSymbol::collectLayoutRules(const nlohmann::json& layoutJson)
{
  if (!layoutJson.is_object())
  {
    return;
  }

  auto& obj = layoutJson[K_OBJ];
  if (!obj.is_array())
  {
    return;
  }

  for (auto& item : obj)
  {
    auto& id = item[K_ID];
    m_layoutRules[id] = item;
  }
}

void ExpandSymbol::traverseElementNode(
  std::shared_ptr<Domain::Element>& element,
  std::vector<std::string>&         instanceIdStack)
{
  if (auto instance = std::dynamic_pointer_cast<Domain::SymbolInstanceElement>(element))
  {
    expandInstanceElement(*instance, instanceIdStack);
    return;
  }

  for (auto& child : element->children())
  {
    traverseElementNode(child, instanceIdStack);
  }
}

void ExpandSymbol::expandInstanceElement(
  Domain::SymbolInstanceElement& instance,
  std::vector<std::string>&      instanceIdStack,
  bool                           again)
{
  const auto instanceId = instance.id(); // copied id; intance id will change
  if (
    std::find(instanceIdStack.begin(), instanceIdStack.end(), instanceId) != instanceIdStack.end())
  {
    FAIL("ExpandSymbol::expandInstance: infinite expanding, a->b->a, return");
    return;
  }

  const auto& masterId = instance.masterId();
  if (m_pMasters.find(masterId) == m_pMasters.end())
  {
    return;
  }

  const auto& master = *m_pMasters[masterId];
  instance.setMaster(master);

  std::shared_ptr<LayoutNode> treeToRebuild;
  // build subtree for recursive expand
  if (instanceIdStack.empty())
  {
    treeToRebuild = m_layout->layoutTree()->findDescendantNodeById(instance.id());
  }
  else
  {
    // find node to rebuild, find in subtree
    treeToRebuild = m_layout->layoutTree();
    std::vector<std::string> tmpIdStack;
    for (auto& tmpId : instanceIdStack)
    {
      tmpIdStack.push_back(tmpId);
      const auto& parentInstanceNodeId = join(tmpIdStack);
      auto        subtreeToRebuild = treeToRebuild->findDescendantNodeById(parentInstanceNodeId);
      if (subtreeToRebuild)
      {
        treeToRebuild = subtreeToRebuild;
      }
      else
      {
        break;
      }
    }
  }

  // rebuild subtree
  if (treeToRebuild)
  {
    m_layout->rebuildSubtree(treeToRebuild);
  }
  else
  {
    DEBUG("ExpandSymbol::expandInstance: node to rebuild not found");
  }

  if (again)
  {
    auto originalId = Helper::split(instanceId).back();
    instanceIdStack.push_back(originalId);
  }
  else
  {
    instanceIdStack.push_back(instanceId);
  }

  // 1. expand
  for (auto& child : instance.children())
  {
    traverseElementNode(child, instanceIdStack);
  }

  // 2 make instance tree nodes id unique
  // 2.1
  if (!again && instanceIdStack.size() > 1)
  {
    std::vector<std::string> idStackWithoutSelf{ instanceIdStack.begin(),
                                                 instanceIdStack.end() - 1 };
    auto                     prefix = join(idStackWithoutSelf) + K_SEPARATOR;
    instance.addKeyPrefix(prefix); // instance id & key changes
  }

  const std::string& instanceIdWithPrefix = instance.id();
  mergeLayoutRule(instanceId, instanceIdWithPrefix);
  mergeLayoutRule(masterId, instanceIdWithPrefix);

  auto idPrefix = instanceIdWithPrefix + K_SEPARATOR;
  for (auto child : instance.children())
  {
    makeTreeKeysUnique(child, idPrefix);
  }

  // 2.2. update mask by: id -> unique id
  for (auto child : instance.children())
  {
    makeMaskIdUnique(child, instance, idPrefix);
  }

  // 3 overrides and scale
  // 3.0 master id refer to a var
  for (auto child : instance.children())
  {
    processVariableRefs(child, instance, instanceIdStack, EProcessVarRefOption::ONLY_MASTER);
  }
  // 3.1 master id overrides
  processMasterIdOverrides(instance, instanceIdStack);

  // 3.2: variables
  processVariableAssignmentsOverrides(instance, instanceIdStack);
  for (auto child : instance.children())
  {
    processVariableRefs(child, instance, instanceIdStack, EProcessVarRefOption::NOT_MASTER);
  }

  // 3.3: layout overrides
  processLayoutOverrides(instance, instanceIdStack);
  // 3.4: scale or layout before bounds overrides
  resizeInstance(instance, master);
  // 3.5 bounds overrides must be processed after scaling
  processBoundsOverrides(instance, instanceIdStack);
  // 3.6 other overrides
  processOtherOverrides(instance, instanceIdStack);
  // 3.7
  layoutDirtyNodes(instance.id());
  instanceIdStack.pop_back();

  // 4. again, makeMaskIdUnique after override: id -> unique id
  for (auto child : instance.children())
  {
    makeMaskIdUnique(child, instance, idPrefix);
  }
}

void ExpandSymbol::applyOverrides(
  nlohmann::json&       json,
  std::string&          name,
  const nlohmann::json& value)
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

  nl::json::json_pointer path{ "/" + name };
  DEBUG(
    "#ExpandSymbol: override object[id=%s, ptr=%p], path=%s, value=%s",
    json[K_ID].get<std::string>().c_str(),
    &json,
    path.to_string().c_str(),
    value.dump().c_str());
  std::stack<std::string> reversedPath;
  while (!path.empty())
  {
    reversedPath.push(path.back());
    path.pop_back();
  }

  applyOverridesDetail(json, reversedPath, value);
}

void ExpandSymbol::applyOverridesDetail(
  nlohmann::json&         json,
  std::stack<std::string> reversedPath,
  const nlohmann::json&   value)
{
  ASSERT(!reversedPath.empty());

  auto key = reversedPath.top();

  reversedPath.pop();
  auto isLastKey = reversedPath.empty();
  if (isLastKey)
  {
    applyLeafOverrides(json, key, value);
    return;
  }

  if (key == "*")
  {
    for (auto& el : json.items())
    {
      applyOverridesDetail(el.value(), reversedPath, value);
    }
  }
  else if (json.is_array())
  {
    auto path = nlohmann::json::json_pointer{ "/" + key };
    if (json.contains(path))
    {
      applyOverridesDetail(json[path], reversedPath, value);
    }
    else
    {
      DEBUG("invalid array index, %s", key.c_str());
    }
  }
  else
  {
    applyOverridesDetail(json[key], reversedPath, value);
  }
}

std::string ExpandSymbol::join(
  const std::vector<std::string>& instanceIdStack,
  const std::string&              separator)
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

void ExpandSymbol::mergeLayoutRule(const std::string& srcId, const std::string& dstId)
{
  if (srcId == dstId)
  {
    return;
  }

  if (!hasOriginalLayoutRule(srcId))
  {
    DEBUG("ExpandSymbol, no src layout rule, %s -x-> %s", srcId.c_str(), dstId.c_str());
    return;
  }

  // merge layout and return
  if (auto dstRulePtr = findOutLayoutObjectById(dstId); dstRulePtr)
  {
    // erased item in json array is null
    auto& dstRule = *dstRulePtr;
    if (dstRule.is_object() && hasOriginalLayoutRule(srcId))
    {
      auto& srcRule = m_layoutRules[srcId];
      if (srcRule.contains(K_LAYOUT)) // copy container layout
      {
        DEBUG("ExpandSymbol, merge layout rule, %s -> %s", srcId.c_str(), dstId.c_str());
        dstRule[K_LAYOUT] = srcRule[K_LAYOUT];
        *(*m_layoutRulesCache)[dstId] = dstRule;
      }
      return;
    }
  }

  // copy layout
  if (hasOriginalLayoutRule(srcId))
  {
    auto dstRule = m_layoutRules[srcId];
    DEBUG("ExpandSymbol, copy layout rule, %s -> %s", srcId.c_str(), dstId.c_str());
    dstRule[K_ID] = dstId;

    auto rule = std::make_shared<Internal::Rule::Rule>();
    *rule = dstRule;
    (*m_layoutRulesCache)[dstId] = rule;

    m_outLayoutJsonMap[dstId] = dstRule;
  }
}

void ExpandSymbol::removeInvalidLayoutRule(const nlohmann::json& json)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.is_object() && json.contains(K_ID))
  {
    std::string id = json[K_ID];
    if (auto dstRulePtr = findOutLayoutObjectById(id); dstRulePtr)
    {
      auto& dstRule = *dstRulePtr;
      dstRule = nlohmann::json(); // assign a null json; same as rules.erase(i);
      m_layoutRulesCache->erase(id);
      m_outLayoutJsonMap.erase(id);
    }
  }

  for (auto& el : json.items())
  {
    removeInvalidLayoutRule(el.value());
  }
}

void ExpandSymbol::overrideLayoutRuleSize(const std::string& instanceId, const Size& instanceSize)
{
  DEBUG(
    "ExpandSymbol::overrideLayoutRuleSize, instanceId: %s, size: %f, %f",
    instanceId.c_str(),
    instanceSize.width,
    instanceSize.height);
  auto dstRulePtr = findOutLayoutObjectById(instanceId);
  if (!dstRulePtr || !dstRulePtr->is_object())
  {
    return;
  }

  auto& dstRule = *dstRulePtr;

  auto ruleCache = (*m_layoutRulesCache)[instanceId];
  if (dstRule[K_WIDTH][K_VALUE][K_TYPES] == Internal::Rule::Length::ETypes::PX)
  {
    dstRule[K_WIDTH][K_VALUE][K_VALUE] = instanceSize.width;
    ruleCache->width.value.value = instanceSize.width;
  }
  else
  {
    DEBUG("ExpandSymbol::overrideLayoutRuleSize: width type is not PX, do not update");
  }

  if (dstRule[K_HEIGHT][K_VALUE][K_TYPES] == Internal::Rule::Length::ETypes::PX)
  {
    dstRule[K_HEIGHT][K_VALUE][K_VALUE] = instanceSize.height;
    ruleCache->height.value.value = instanceSize.height;
  }
  else
  {
    DEBUG("ExpandSymbol::overrideLayoutRuleSize: height type is not PX, do not update");
  }
}

void ExpandSymbol::layoutSubtree(const std::string& subtreeNodeId, Size size, bool preservingOrigin)
{
  m_layout->resizeNodeThenLayout(std::string{ subtreeNodeId }, size, preservingOrigin);
  overrideLayoutRuleSize(subtreeNodeId, size);
}

void ExpandSymbol::layoutSubtree(
  std::shared_ptr<LayoutNode> subtreeNode,
  Size                        size,
  bool                        preservingOrigin)
{
  ASSERT(subtreeNode);
  m_layout->resizeNodeThenLayout(subtreeNode, size, preservingOrigin);
  overrideLayoutRuleSize(subtreeNode->id(), size);
}

void ExpandSymbol::layoutDirtyNodes(const std::string& instanceId)
{
  if (m_tmpDirtyNodeIds.empty())
  {
    return;
  }

  m_layout->layoutNodes(m_tmpDirtyNodeIds, instanceId);

  m_tmpDirtyNodeIds.clear();
}

nlohmann::json* ExpandSymbol::findOutLayoutObjectById(const std::string& id)
{
  if (m_outLayoutJsonMap.find(id) != m_outLayoutJsonMap.end())
  {
    return &m_outLayoutJsonMap[id];
  }
  else
  {
    return nullptr;
  }
}

void ExpandSymbol::applyLeafOverrides(
  nlohmann::json&       json,
  const std::string&    key,
  const nlohmann::json& value)
{
  if (value.is_null()) // A null value indicates deletion of the element
  {
    deleteLeafElement(json, key);
    return;
  }

  if (key == "*")
  {
    for (auto& el : json.items())
    {
      el.value() = value;
    }
  }
  else if (json.is_array())
  {
    auto path = nlohmann::json::json_pointer{ "/" + key };
    if (json.contains(path))
    {
      json[path] = value;
    }
    else
    {
      DEBUG("invalid array index, %s", key.c_str());
    }
  }
  else
  {
    json[key] = value;

    if (key == K_VISIBLE)
    {
      DEBUG(
        "applyLeafOverrides, node's visible changed, add to dirty list, %s",
        json[K_ID].dump().c_str());
      m_tmpDirtyNodeIds.push_back(json[K_ID]);
    }
  }
}

bool ExpandSymbol::hasOriginalLayoutRule(const std::string& id)
{
  return m_layoutRules.find(id) != m_layoutRules.end();
}

ExpandSymbol::RuleMapPtr ExpandSymbol::getLayoutRules()
{
  return m_layoutRulesCache;
}

nlohmann::json ExpandSymbol::generateOutLayoutJson()
{
  if (!m_layoutJson.is_object())
  {
    return m_layoutJson;
  }

  nlohmann::json result(nlohmann::json::value_t::object);
  result[K_OBJ] = nlohmann::json(nlohmann::json::value_t::array);
  for (auto& [key, value] : m_outLayoutJsonMap)
  {
    if (value.is_object())
    {
      result[K_OBJ].push_back(value);
    }
  }

  return result;
}

void ExpandSymbol::makeTreeKeysUnique(
  std::shared_ptr<Domain::Element>& element,
  const std::string&                idPrefix)
{
  if (!element)
  {
    return;
  }

  if (element && !element->id().empty())
  {
    // skip expanded instance
    if (auto instance = std::dynamic_pointer_cast<SymbolInstanceElement>(element))
    {
      return;
    }

    auto oldObjectId = element->id();
    element->addKeyPrefix(idPrefix);

    auto newObjectId = element->id();
    mergeLayoutRule(oldObjectId, newObjectId);
  }

  for (auto child : element->children())
  {
    makeTreeKeysUnique(child, idPrefix);
  }
}

void ExpandSymbol::makeMaskIdUnique(
  std::shared_ptr<Domain::Element>& element,
  Domain::SymbolInstanceElement&    instance,
  const std::string&                idPrefix)
{
  if (!element)
  {
    return;
  }

  element->makeMaskIdUnique(instance, idPrefix);
}

void ExpandSymbol::processVariableRefs(
  std::shared_ptr<Domain::Element>& element, // in instance tree
  Domain::Element&                  container,
  const std::vector<std::string>&   instanceIdStack,
  EProcessVarRefOption              option)
{
  if (!element)
  {
    return;
  }

  do
  {
    if (!element->isLayoutNode())
    {
      break;
    }
    if (!element->model())
    {
      break;
    }
    if (!element->model()->variableRefs)
    {
      break;
    }
    auto& refs = element->model()->variableRefs.value();

    const std::vector<VariableAssign>* containerVarAssigns = nullptr;
    if (container.type() == Element::EType::SYMBOL_INSTANCE)
    {
      auto containerInstance = static_cast<SymbolInstanceElement*>(&container);
      if (containerInstance->model() && containerInstance->model()->variableAssignments)
      {
        containerVarAssigns = &containerInstance->model()->variableAssignments.value();
      }
    }

    const std::vector<VariableDefine>* containerVarDefs = nullptr;
    if (container.model() && container.model()->variableDefs)
    {
      containerVarDefs = &container.model()->variableDefs.value();
    }

    const std::vector<VariableDefine>* myVarDefs = nullptr;
    if (element->model() && element->model()->variableDefs)
    {
      myVarDefs = &element->model()->variableDefs.value();
    }

    for (auto& ref : refs)
    {
      auto& varId = ref.id;
      auto& objectField = ref.objectField;

      if (option == EProcessVarRefOption::ONLY_MASTER)
      {
        if (objectField != K_MASTER_ID)
        {
          continue;
        }
      }
      else if (option == EProcessVarRefOption::NOT_MASTER)
      {
        if (objectField == K_MASTER_ID)
        {
          continue;
        }
      }

      const nlohmann::json* value{ nullptr };
      auto                  varType{ EVarType::NONE };
      auto                  found{ false };

      // find var value
      // find in container assignments
      if (containerVarAssigns)
      {
        for (auto& assign : *containerVarAssigns)
        {
          if (varId == assign.id)
          {
            found = true;
            value = &assign.value;
            if (objectField == K_TEXT_DATA)
            {
              varType = EVarType::TEXT_PROPERTY;
            }
            // else varType is bool or string
            break;
          }
        }
      }

      if (!found && myVarDefs)
      {
        // find in own defs
        for (auto& def : *myVarDefs)
        {
          if (varId == def.id)
          {
            value = &def.value;
            varType = EVarType{ def.varType };
            found = true;
            break;
          }
        }
      }

      if (!found && containerVarDefs)
      {
        // find in container defs
        for (auto& def : *containerVarDefs)
        {
          if (varId == def.id)
          {
            value = &def.value;
            varType = EVarType{ def.varType };
            found = true;
            break;
          }
        }
      }

      // apply value
      if (found)
      {
        if (varType == EVarType::REFERENCE)
        {
          DEBUG("processVariableRefs, var type is reference");
        }
        else if (varType == EVarType::TEXT_PROPERTY)
        {
          auto textElement = std::dynamic_pointer_cast<TextElement>(element);
          if (textElement)
          {
            textElement->updateFields(*value);
          }
        }
        else
        {
          if (objectField == K_MASTER_ID) // masterId
          {
            auto instance = std::dynamic_pointer_cast<SymbolInstanceElement>(element);
            if (instance)
            {
              resetInstanceInfo(*instance);
              instance->updateMasterId(*value);
              auto childInstanceIdStack = instanceIdStack;
              expandInstanceElement(*instance, childInstanceIdStack, true);
            }
          }
          else if (objectField == K_VISIBLE)
          {
            bool visible = *value;
            element->setVisible(visible);
          }
        }
      }
    }

  } while (false);

  for (auto child : element->children())
  {
    processVariableRefs(child, *element, instanceIdStack, option);
  }
}

void ExpandSymbol::resetInstanceInfo(SymbolInstanceElement& instance)
{
  // Keep own layout rule; Remove children layout rule only;
  removeInvalidLayoutRule(instance, true);

  if (auto node = m_layout->layoutTree()->findDescendantNodeById(instance.id()))
  {
    node->removeAllChildren(); // remove all children
  }
}

void ExpandSymbol::removeInvalidLayoutRule(const Element& element, bool keepOwn)
{
  if (!keepOwn)
  {
    auto id = element.id();
    if (auto dstRulePtr = findOutLayoutObjectById(id); dstRulePtr)
    {
      auto& dstRule = *dstRulePtr;
      dstRule = nlohmann::json(); // assign a null json; same as rules.erase(i);
      m_layoutRulesCache->erase(id);
      m_outLayoutJsonMap.erase(id);
    }
  }

  for (auto child : element.children())
  {
    removeInvalidLayoutRule(*child);
  }
}

void ExpandSymbol::processMasterIdOverrides(
  Domain::SymbolInstanceElement&  instance,
  const std::vector<std::string>& instanceIdStack)
{
  auto instanceModel = instance.model();
  if (!instanceModel)
  {
    return;
  }

  std::vector<OverrideValue> masterIdOverrideValues;
  std::copy_if(
    instanceModel->overrideValues.begin(),
    instanceModel->overrideValues.end(),
    std::back_inserter(masterIdOverrideValues),
    [](const OverrideValue& item) { return item.overrideName == K_MASTER_ID; });

  // Sorting: Top-down, root first;
  std::stable_sort(
    masterIdOverrideValues.begin(),
    masterIdOverrideValues.end(),
    [](const OverrideValue& a, const OverrideValue& b)
    { return a.objectId.size() < b.objectId.size(); });

  for (auto& overrideItem : masterIdOverrideValues)
  {
    std::vector<std::string> childInstanceIdStack;
    auto                     childObject =
      findChildObject(instance, instanceIdStack, overrideItem, childInstanceIdStack);
    if (!childObject)
    {
      continue;
    }

    auto childInstance = std::dynamic_pointer_cast<SymbolInstanceElement>(childObject);
    if (!childInstance)
    {
      continue;
    }

    resetInstanceInfo(*childInstance);
    childInstance->updateMasterId(overrideItem.overrideValue);
    expandInstanceElement(*childInstance, childInstanceIdStack, true);
  }
}

std::shared_ptr<Element> ExpandSymbol::findChildObject(
  Domain::SymbolInstanceElement&  instance,
  const std::vector<std::string>& instanceIdStack,
  const Model::OverrideValue&     overrideItem,
  std::vector<std::string>&       outChildInstanceIdStack)
{
  auto& objectIdPaths = overrideItem.objectId;
  if (objectIdPaths.empty())
  {
    return nullptr;
  }

  if (objectIdPaths.size() == 1)
  {
    if (
      objectIdPaths.front() == instance.masterId() ||
      objectIdPaths.front() == instance.masterOverrideKey())
    {
      outChildInstanceIdStack = instanceIdStack;
      return instance.shared_from_this();
    }
  }

  outChildInstanceIdStack = instanceIdStack;
  return instance.findElementByKey(objectIdPaths, &outChildInstanceIdStack);
}

void ExpandSymbol::processVariableAssignmentsOverrides(
  Domain::SymbolInstanceElement&  instance,
  const std::vector<std::string>& instanceIdStack)
{
  auto instanceModel = instance.model();
  if (!instanceModel)
  {
    return;
  }

  std::vector<VGG::Model::OverrideValue> variableAssignments;
  std::copy_if(
    instanceModel->overrideValues.begin(),
    instanceModel->overrideValues.end(),
    std::back_inserter(variableAssignments),
    [](const OverrideValue& item) { return item.overrideName == K_VARIABLE_ASSIGNMENTS; });

  for (auto& overrideItem : variableAssignments)
  {
    std::vector<std::string> _;
    auto childObject = findChildObject(instance, instanceIdStack, overrideItem, _);
    if (!childObject)
    {
      continue;
    }

    auto childInstance = std::dynamic_pointer_cast<SymbolInstanceElement>(childObject);
    if (!childInstance)
    {
      continue;
    }

    childInstance->updateVariableAssignments(overrideItem.overrideValue);
    for (auto& child : childInstance->children())
    {
      processVariableRefs(child, *childInstance, _, EProcessVarRefOption::ALL);
    }
  }
}

void ExpandSymbol::processLayoutOverrides(
  Domain::SymbolInstanceElement&  instance,
  const std::vector<std::string>& instanceIdStack)
{
  auto instanceModel = instance.model();
  if (!instanceModel)
  {
    return;
  }

  std::vector<OverrideValue> layoutOverrideValues;
  std::copy_if(
    instanceModel->overrideValues.begin(),
    instanceModel->overrideValues.end(),
    std::back_inserter(layoutOverrideValues),
    [](const OverrideValue& item) { return item.effectOnLayout; });

  for (auto& overrideItem : layoutOverrideValues)
  {
    std::vector<std::string> _;

    auto element = findChildObject(instance, instanceIdStack, overrideItem, _);
    if (!element)
    {
      continue;
    }
    auto layoutObject = findOutLayoutObjectById(element->id());
    if (!layoutObject)
    {
      continue;
    }

    std::string path = overrideItem.overrideName;
    auto        value = overrideItem.overrideValue;
    applyOverrides((*layoutObject), path, value);

    if (layoutObject->contains(K_ID))
    {
      *(*m_layoutRulesCache)[(*layoutObject)[K_ID]] = *layoutObject;
    }
  }
}

void ExpandSymbol::resizeInstance(
  Domain::SymbolInstanceElement&     instance,
  const Domain::SymbolMasterElement& master)
{
  auto instanceModel = instance.model();
  if (!instanceModel)
  {
    return;
  }
  auto masterModel = master.model();
  if (!masterModel)
  {
    return;
  }

  Rect masterBounds{ { masterModel->bounds.x, masterModel->bounds.y },
                     { masterModel->bounds.width, masterModel->bounds.height } };
  Size instanceSize{ instanceModel->bounds.width, instanceModel->bounds.height };
  if (masterBounds.size == instanceSize)
  {
    return;
  }
  instance.updateBounds(masterBounds);
  layoutInstance(instance, instanceSize);
}

void ExpandSymbol::layoutInstance(Domain::SymbolInstanceElement& instance, const Size& instanceSize)
{
  auto node = m_layout->layoutTree()->findDescendantNodeById(instance.id());
  if (!node)
  {
    return;
  }

  m_layout->rebuildSubtree(node); // force rebuild subtree with updated rules

  node->setNeedLayout();

  // layout with new size
  layoutSubtree(node, instanceSize, true);
}

void ExpandSymbol::processBoundsOverrides(
  Domain::SymbolInstanceElement&  instance,
  const std::vector<std::string>& instanceIdStack)
{
  auto instanceModel = instance.model();
  if (!instanceModel)
  {
    return;
  }

  std::vector<OverrideValue> boundsOverrideValues;
  std::copy_if(
    instanceModel->overrideValues.begin(),
    instanceModel->overrideValues.end(),
    std::back_inserter(boundsOverrideValues),
    [](const OverrideValue& item) { return item.overrideName == K_BOUNDS; });

  // Sorting: Top-down, root first;
  std::stable_sort(
    boundsOverrideValues.begin(),
    boundsOverrideValues.end(),
    [self = this, &instance, &instanceIdStack](const OverrideValue& a, const OverrideValue& b)
    {
      if (a.objectId.size() == b.objectId.size())
      {
        std::vector<std::string> _;

        auto aElement = self->findChildObject(instance, instanceIdStack, a, _);
        auto bElement = self->findChildObject(instance, instanceIdStack, b, _);
        if (aElement && bElement)
        {
          return aElement->isAncestorOf(bElement);
        }

        return false;
      }
      else
      {
        return a.objectId.size() < b.objectId.size();
      }
    });

  for (auto& overrideItem : boundsOverrideValues)
  {
    std::vector<std::string> _;
    auto                     element = findChildObject(instance, instanceIdStack, overrideItem, _);
    if (!element)
    {
      continue;
    }

    Rect newBounds = overrideItem.overrideValue;
    layoutSubtree(element->id(), newBounds.size, false);
  }
}

void ExpandSymbol::processOtherOverrides(
  Domain::SymbolInstanceElement&  instance,
  const std::vector<std::string>& instanceIdStack)
{
  auto instanceModel = instance.model();
  if (!instanceModel)
  {
    return;
  }

  const auto copiedItems = instanceModel->overrideValues; // copy to avoid iterator invalidation
  for (auto& item : copiedItems)
  {
    // skip items handled before
    if (
      item.effectOnLayout.value_or(false) || item.overrideName == K_BOUNDS ||
      item.overrideName == K_MASTER_ID || item.overrideName == K_VARIABLE_ASSIGNMENTS ||
      item.overrideName.empty())
    {
      continue;
    }

    std::vector<std::string> _;

    auto element = findChildObject(instance, instanceIdStack, item, _);
    if (!element)
    {
      continue;
    }

    auto& value = item.overrideValue;
    if (applyReferenceOverride(*element, item.overrideName, value))
    {
      continue;
    }
    element->applyOverride(item.overrideName, value, m_tmpDirtyNodeIds);
  }
}

bool ExpandSymbol::applyReferenceOverride(
  Domain::Element&      element,
  const std::string&    name,
  const nlohmann::json& value)
{
  if (name != K_STYLE)
  {
    return false;
  }
  if (!value.is_string())
  {
    return false;
  }

  std::string s = value;
  if (s.rfind(K_PREFIX, 0) != 0)
  {
    return false;
  }

  auto id = s.substr(std::strlen(K_PREFIX));
  if (m_designModel->references)
  {
    auto& items = *m_designModel->references;
    for (auto& srcReferenence : items)
    {
      if (auto p = std::get_if<ReferencedStyle>(&srcReferenence))
      {
        if (p->id == id)
        {
          element.update(*p);

          return true;
        }
      }
    }
    return true;
  }

  return false;
}

void ExpandSymbol::expandInstance(
  std::shared_ptr<Domain::SymbolInstanceElement> instance,
  std::string                                    masterId)
{
  DEBUG(
    "ExpandSymbol::expandInstance %s with master id %s",
    instance->id().c_str(),
    masterId.c_str());

  if (!instance || masterId.empty())
  {
    return;
  }

  resetInstanceInfo(*instance);
  instance->updateMasterId(masterId);

  std::vector<std::string>         instanceIdStack;
  std::shared_ptr<Domain::Element> tmpElement = instance->parent();
  while (tmpElement)
  {
    if (tmpElement->type() == Element::EType::SYMBOL_INSTANCE)
    {
      instanceIdStack.insert(instanceIdStack.begin(), tmpElement->id());
    }
    tmpElement = tmpElement->parent();
  }

  expandInstanceElement(*instance, instanceIdStack, true);
}