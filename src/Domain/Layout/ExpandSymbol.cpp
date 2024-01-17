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
#include "ExpandSymbol.hpp"

#include "Layout.hpp"
#include "ReferenceJsonDocument.hpp"
#include "Rule.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Utility/Log.hpp"

#include <algorithm>
#include <fstream>
#include <numeric>
#include <iostream>

#undef DEBUG
#define DEBUG(msg, ...)

// #define DUMP_TREE

constexpr auto K_PREFIX = "referenced_style_";
constexpr auto K_BORDER_PREFIX = "style.borders";

namespace nl = nlohmann;
using namespace VGG::Layout;

namespace
{
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

bool isAncestor(const nlohmann::json& ancestor, const nlohmann::json& descendant)
{
  if (&ancestor == &descendant)
  {
    return true;
  }

  if (ancestor.is_array() || ancestor.is_object())
  {
    for (auto& el : ancestor.items())
    {
      if (isAncestor(el.value(), descendant))
      {
        return true;
      }
    }
  }

  return false;
}

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

nlohmann::json ExpandSymbol::operator()()
{
  return std::get<0>(run());
}

std::pair<nlohmann::json, nlohmann::json> ExpandSymbol::run()
{
  collectMaster(m_designJson);
  collectLayoutRules(m_layoutJson);

  m_tmpOutDesignJson = m_designJson;
  m_layoutRulesCache = Layout::collectRules(m_layoutJson);
  m_outLayoutJsonMap = m_layoutRules;

  m_layout.reset(new Layout{ JsonDocumentPtr{ new ReferenceJsonDocument{ m_tmpOutDesignJson } },
                             getLayoutRules() });

  std::vector<std::string> instanceIdStack{};
  expandInstance(m_tmpOutDesignJson, instanceIdStack);

  auto outLayoutJson = generateOutLayoutJson();

  return { std::move(m_tmpOutDesignJson), std::move(outLayoutJson) };
}

void ExpandSymbol::collectMaster(const nlohmann::json& json)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.is_object())
  {
    auto className = json.value(K_CLASS, K_EMPTY_STRING);
    if (className == K_SYMBOL_MASTER)
    {
      auto id = json[K_ID];
      m_masters[id] = json;
    }
  }

  for (auto& el : json.items())
  {
    collectMaster(el.value());
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

void ExpandSymbol::expandInstance(
  nlohmann::json&           json,
  std::vector<std::string>& instanceIdStack,
  bool                      again)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.is_object())
  {
    auto className = json.value(K_CLASS, K_EMPTY_STRING);
    if (className == K_SYMBOL_INSTANCE)
    {
      if (!json.contains(K_MASTER_ID))
      {
        WARN("ExpandSymbol::expandInstance: no master id, return");
        return;
      }

      auto instanceId = json[K_ID].get<std::string>();
      if (
        std::find(instanceIdStack.begin(), instanceIdStack.end(), instanceId) !=
        instanceIdStack.end())
      {
        FAIL("ExpandSymbol::expandInstance: infinite expanding, a->b->a, return");
        return;
      }

      auto masterId = json[K_MASTER_ID].get<std::string>();
      if (m_masters.find(masterId) != m_masters.end())
      {
        DEBUG(
          "#ExpandSymbol: expand instance[id=%s, ptr=%p] in stack [%s] with masterId=%s",
          instanceId.c_str(),
          &json,
          join(instanceIdStack, ", ").c_str(),
          masterId.c_str());
        auto& masterJson = m_masters[masterId];
        json[K_CHILD_OBJECTS] = masterJson[K_CHILD_OBJECTS];
        json[K_STYLE] = masterJson[K_STYLE];
        json[K_VARIABLE_DEFS] = masterJson[K_VARIABLE_DEFS];

        // build subtree for recursive expand
        if (instanceIdStack.empty())
        {
          m_layout->rebuildSubtreeById(json[K_ID]);
        }
        else
        {
          std::shared_ptr<LayoutNode> nodeToRebuild;

          // find node to rebuild
          auto tmpInstanceIdStack = instanceIdStack;
          while (!tmpInstanceIdStack.empty())
          {
            auto parentInstanceNodeId = join(tmpInstanceIdStack);
            nodeToRebuild = m_layout->layoutTree()->findDescendantNodeById(parentInstanceNodeId);
            if (nodeToRebuild)
            {
              break;
            }

            tmpInstanceIdStack.pop_back();
          }

          // rebuild subtree
          if (nodeToRebuild)
          {
            m_layout->rebuildSubtree(nodeToRebuild);
          }
          else
          {
            DEBUG("ExpandSymbol::expandInstance: node to rebuild not found");
          }
        }

        if (again)
        {
          auto originalId = split(instanceId).back();
          instanceIdStack.push_back(originalId);
        }
        else
        {
          instanceIdStack.push_back(instanceId);
        }

        // 1. expand
        expandInstance(json[K_CHILD_OBJECTS], instanceIdStack);

        // 2 make instance tree nodes id unique
        // 2.1
        if (!again && instanceIdStack.size() > 1)
        {
          std::vector<std::string> idStackWithoutSelf{ instanceIdStack.begin(),
                                                       instanceIdStack.end() - 1 };
          auto                     prefix = join(idStackWithoutSelf) + K_SEPARATOR;
          makeNodeKeysUnique(json, prefix);
        }

        std::string instanceIdWithPrefix = json[K_ID];
        mergeLayoutRule(instanceId, instanceIdWithPrefix);
        mergeLayoutRule(masterId, instanceIdWithPrefix);

        auto idPrefix = instanceIdWithPrefix + K_SEPARATOR;
        makeTreeKeysUnique(json[K_CHILD_OBJECTS], idPrefix); // children

        // 2.2. update mask by: id -> unique id
        makeMaskIdUnique(json[K_CHILD_OBJECTS], json, idPrefix);

        // 3 overrides and scale
        // 3.1 master id overrides
        processMasterIdOverrides(json, instanceIdStack);

        // 3.2: variables
        // process variableAssignments overrides
        processVariableAssignmentsOverrides(json, instanceIdStack);
        // process variableRefs
        for (auto& child : json[K_CHILD_OBJECTS])
        {
          processVariableRefs(child, &json, instanceIdStack);
        }

        // 3.3: layout overrides
        processLayoutOverrides(json, instanceIdStack);
        // 3.4: scale or layout before bounds overrides
        resizeInstance(json, masterJson);
        // 3.5 bounds overrides must be processed after scaling
        processBoundsOverrides(json, instanceIdStack);
        // 3.6 other overrides
        processOtherOverrides(json, instanceIdStack);
        // 3.7
        layoutDirtyNodes(json);
        instanceIdStack.pop_back();

        // 4. again, makeMaskIdUnique after override: id -> unique id
        makeMaskIdUnique(json[K_CHILD_OBJECTS], json, idPrefix);

        // 5. make instance node to "symbalMaster" or render will not draw this node
        DEBUG(
          "#ExpandSymbol: make instance[id=%s, ptr=%p] as master, erase masterId",
          instanceId.c_str(),
          &json);
        json[K_CLASS] = K_SYMBOL_MASTER;
        json.erase(K_MASTER_ID);
        json.erase(K_OVERRIDE_VALUES);
      }

      return;
    }
  }

  for (auto& el : json.items())
  {
    expandInstance(el.value(), instanceIdStack);
  }
}

void ExpandSymbol::resizeInstance(nlohmann::json& instance, nlohmann::json& master)
{
  auto masterBounds = master[K_BOUNDS].get<Rect>();
  auto instanceBounds = instance[K_BOUNDS].get<Rect>();

  // todo: handle translate if masterBounds.origin != instanceBounds.origin
  if (masterBounds.size == instanceBounds.size)
  {
    return;
  }

  to_json(instance[K_BOUNDS], masterBounds); // use master's bounds for layout
  layoutInstance(instance, instanceBounds.size);
  to_json(instance[K_BOUNDS], instanceBounds); // restore instance bounds

  // todo: handle translate if masterBounds.origin != instanceBounds.origin
}

void ExpandSymbol::processMasterIdOverrides(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack)
{
  const auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto masterIdOverrideValues = nlohmann::json::array();
  std::copy_if(
    overrideValues.begin(),
    overrideValues.end(),
    std::back_inserter(masterIdOverrideValues),
    [](const nlohmann::json& item) { return item[K_OVERRIDE_NAME] == K_MASTER_ID; });

  // Sorting: Top-down, root first;
  std::stable_sort(
    masterIdOverrideValues.begin(),
    masterIdOverrideValues.end(),
    [](const nlohmann::json& a, const nlohmann::json& b)
    {
      auto& aObjectIdPaths = a[K_OBJECT_ID];
      auto& bObjectIdPaths = b[K_OBJECT_ID];
      return aObjectIdPaths.size() < bObjectIdPaths.size();
    });

  for (auto& el : masterIdOverrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }

    std::vector<std::string> childInstanceIdStack;
    auto                     childObject =
      findChildObject(instance, instanceIdStack, overrideItem, childInstanceIdStack);
    if (!childObject || !childObject->is_object())
    {
      continue;
    }

    auto value = overrideItem[K_OVERRIDE_VALUE];

    DEBUG(
      "#ExpandSymbol: overide instance[id=%s, ptr=%p], old masterId=%s, new masterId=%s, restore "
      "class to symbolInstance to expand",
      (*childObject)[K_ID].dump().c_str(),
      childObject,
      (*childObject)[K_MASTER_ID].dump().c_str(),
      value.get<std::string>().c_str());

    (*childObject)[K_MASTER_ID] = value;
    resetInstanceInfo(*childObject);
    expandInstance(*childObject, childInstanceIdStack, true);
  }
}

void ExpandSymbol::processVariableAssignmentsOverrides(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack)
{
  const auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto varAssignsOverrideValues = nlohmann::json::array();
  std::copy_if(
    overrideValues.begin(),
    overrideValues.end(),
    std::back_inserter(varAssignsOverrideValues),
    [](const nlohmann::json& item) { return item[K_OVERRIDE_NAME] == K_VARIABLE_ASSIGNMENTS; });

  for (auto& el : varAssignsOverrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }

    std::vector<std::string> _;
    auto childObject = findChildObject(instance, instanceIdStack, overrideItem, _);
    if (!childObject || !childObject->is_object())
    {
      continue;
    }

    std::string name{ K_VARIABLE_ASSIGNMENTS };
    auto&       value = overrideItem[K_OVERRIDE_VALUE];
    applyOverrides((*childObject), name, value);
    for (auto& child : childObject->at(K_CHILD_OBJECTS))
    {
      // process var refs
      processVariableRefs(child, childObject, _);
    }
  }
}

void ExpandSymbol::processVariableRefs(
  nlohmann::json&                 node,      // in instance tree
  nlohmann::json*                 container, // instance; master set; parent node
  const std::vector<std::string>& instanceIdStack)
{
  if (!node.is_object() && !node.is_array())
  {
    return;
  }

  if (isLayoutNode(node))
  {
    auto* containerVarAssigns = container && container->contains(K_VARIABLE_ASSIGNMENTS)
                                  ? &(*container)[K_VARIABLE_ASSIGNMENTS]
                                  : nullptr;
    auto* containerVarDefs = container ? &(*container)[K_VARIABLE_DEFS] : nullptr;
    auto& myVarDefs = node[K_VARIABLE_DEFS];

    auto& refs = node[K_VARIABLE_REFS];
    for (auto& ref : refs)
    {
      // get var id
      auto&       varId = ref[K_ID];
      std::string objectField = ref[K_OBJECT_FIELD];

      nlohmann::json* value{ nullptr };
      auto            varType{ EVarType::NONE };
      auto            found{ false };

      // find var value
      // find in container assignments
      if (containerVarAssigns)
      {
        for (auto& assign : *containerVarAssigns)
        {
          if (varId == assign[K_ID])
          {
            found = true;
            value = &assign[K_VALUE];
            if (objectField == K_TEXT_DATA)
            {
              varType = EVarType::TEXT_PROPERTY;
            }
            // else varType is bool or string
            break;
          }
        }
      }

      if (!found)
      {
        // find in own defs
        for (auto& def : myVarDefs)
        {
          if (varId == def[K_ID])
          {
            value = &def[K_VALUE];
            varType = def[K_VAR_TYPE];
            // todo? proecess reference type
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
          if (varId == def[K_ID])
          {
            value = &def[K_VALUE];
            varType = def[K_VAR_TYPE];
            // todo? proecess reference type
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
          for (auto& el : (*value).items())
          {
            node[el.key()] = el.value();
          }
        }
        else
        {
          node[objectField] = *value;
          if (objectField == K_MASTER_ID) // masterId
          {
            DEBUG(
              "#ExpandSymbol::processVariableRefs: overide instance[id=%s, ptr=%p], "
              "new masterId=%s, restore class to symbolInstance to expand",
              node[K_ID].dump().c_str(),
              &node,
              node[K_MASTER_ID].dump().c_str());

            resetInstanceInfo(node);
            auto childInstanceIdStack = instanceIdStack;
            expandInstance(node, childInstanceIdStack, true);
          }
        }
      }
    }
  }

  for (auto& el : node.items())
  {
    processVariableRefs(el.value(), container, instanceIdStack);
  }
}

void ExpandSymbol::processLayoutOverrides(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack)
{
  const auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto layoutOverrideValues = nlohmann::json::array();
  std::copy_if(
    overrideValues.begin(),
    overrideValues.end(),
    std::back_inserter(layoutOverrideValues),
    [](const nlohmann::json& item) { return item.value(K_EFFECT_ON_LAYOUT, false); });

  for (auto& el : layoutOverrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }

    auto layoutObject = findOutLayoutObject(instance, instanceIdStack, overrideItem);
    if (!layoutObject || !layoutObject->is_object())
    {
      continue;
    }

    std::string path = overrideItem[K_OVERRIDE_NAME];
    auto        value = overrideItem[K_OVERRIDE_VALUE];
    applyOverrides((*layoutObject), path, value);

    if (layoutObject->contains(K_ID))
    {
      *(*m_layoutRulesCache)[(*layoutObject)[K_ID]] = *layoutObject;
    }
    else
    {
      WARN("ExpandSymbol::processLayoutOverrides: layout object has no id");
    }
  }
}

void ExpandSymbol::processBoundsOverrides(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack)
{
  const auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto boundsOverrideValues = nlohmann::json::array();
  std::copy_if(
    overrideValues.begin(),
    overrideValues.end(),
    std::back_inserter(boundsOverrideValues),
    [](const nlohmann::json& item) { return item[K_OVERRIDE_NAME] == K_BOUNDS; });

  // Sorting: Top-down, root first;
  std::stable_sort(
    boundsOverrideValues.begin(),
    boundsOverrideValues.end(),
    [self = this, &instance, &instanceIdStack](const nlohmann::json& a, const nlohmann::json& b)
    {
      auto& aObjectIdPaths = a[K_OBJECT_ID];
      auto& bObjectIdPaths = b[K_OBJECT_ID];
      if (aObjectIdPaths.size() == bObjectIdPaths.size())
      {
        std::vector<std::string> _;
        auto aChildObject = self->findChildObject(instance, instanceIdStack, a, _);
        auto bChildObject = self->findChildObject(instance, instanceIdStack, b, _);

        // a is b's ancestor
        if (aChildObject && bChildObject)
        {
          return isAncestor(*aChildObject, *bChildObject);
        }

        return false;
      }
      else
      {
        return aObjectIdPaths.size() < bObjectIdPaths.size();
      }
    });

  for (auto& el : boundsOverrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }

    std::vector<std::string> _;
    auto childObject = findChildObject(instance, instanceIdStack, overrideItem, _);
    if (!childObject || !childObject->is_object())
    {
      continue;
    }

    auto value = overrideItem[K_OVERRIDE_VALUE];
    resizeSubtree(*childObject, value);
  }
}

void ExpandSymbol::processOtherOverrides(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack)
{
  auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  for (auto& el : overrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }
    if (overrideItem.value(K_EFFECT_ON_LAYOUT, false))
    {
      continue;
    }

    std::vector<std::string> _;
    auto childObject = findChildObject(instance, instanceIdStack, overrideItem, _);
    if (!childObject || !childObject->is_object())
    {
      continue;
    }

    std::string name = overrideItem[K_OVERRIDE_NAME];
    if (name == K_MASTER_ID || name == K_BOUNDS || name == K_VARIABLE_ASSIGNMENTS || name.empty())
    {
      continue;
    }

    auto& value = overrideItem[K_OVERRIDE_VALUE];
    if (applyReferenceOverride(*childObject, name, value))
    {
      continue;
    }

    applyOverrides((*childObject), name, value);
  }
}

void ExpandSymbol::applyOverrides(
  nlohmann::json&       json,
  std::string&          name,
  const nlohmann::json& value)
{
  const auto isBorder = name.rfind(K_BORDER_PREFIX, 0) == 0;

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

  if (isBorder && isVectorNetworkGroupNode(json))
  {
    applyOverridesDetailToTree(json, reversedPath, value);
  }
  else
  {
    applyOverridesDetail(json, reversedPath, value);
  }
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

void ExpandSymbol::applyOverridesDetailToTree(
  nlohmann::json&         json,
  std::stack<std::string> reversedPath,
  const nlohmann::json&   value)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  for (auto& el : json.items())
  {
    applyOverridesDetailToTree(el.value(), reversedPath, value);
  }

  if (!isLayoutNode(json))
  {
    return;
  }

  applyOverridesDetail(json, reversedPath, value);
}

nlohmann::json* ExpandSymbol::findChildObjectInTree(
  nlohmann::json&                 json,
  const std::vector<std::string>& keyStack,
  std::vector<std::string>*       outInstanceIdStack)
{
  if (!json.is_object() && !json.is_array())
  {
    return nullptr;
  }

  if (keyStack.empty())
  {
    return nullptr;
  }

  if (json.is_object())
  {
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
    if (const auto& firstObjectId = join(tmpInstanceIdStack);
        (json.contains(K_OVERRIDE_KEY) && json[K_OVERRIDE_KEY] == firstObjectId) ||
        (json.contains(K_ID) && json[K_ID] == firstObjectId))
    {
      if (keyStack.size() == 1) // is last key
      {
        return &json;
      }
      else
      {
        // the id is already prefixed: xxx__yyy__zzz;
        const auto originalId = split(json[K_ID]).back();
        theOutInstanceIdStack->push_back(originalId);

        return findChildObjectInTree(
          json,
          { keyStack.begin() + 1, keyStack.end() },
          theOutInstanceIdStack);
      }
    }
  }

  for (auto& el : json.items())
  {
    auto target = findChildObjectInTree(el.value(), keyStack, outInstanceIdStack);
    if (target)
    {
      return target;
    }
  }

  return nullptr;
}

void ExpandSymbol::makeTreeKeysUnique(nlohmann::json& json, const std::string& idPrefix)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (isLayoutNode(json))
  {
    // skip expanded instance
    auto className = json.value(K_CLASS, K_EMPTY_STRING);
    if (className == K_SYMBOL_MASTER)
    {
      return;
    }

    auto oldObjectId = json[K_ID].get<std::string>();
    makeNodeKeysUnique(json, idPrefix);

    auto newObjectId = json[K_ID].get<std::string>();
    mergeLayoutRule(oldObjectId, newObjectId);
  }

  for (auto& el : json.items())
  {
    makeTreeKeysUnique(el.value(), idPrefix);
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

void ExpandSymbol::makeMaskIdUnique(
  nlohmann::json&    json,
  nlohmann::json&    instanceJson,
  const std::string& idPrefix)
{
  if (json.is_object())
  {
    if (!isLayoutNode(json))
    {
      return;
    }
    DEBUG(
      "ExpandSymbol::makeMaskIdUnique: visit node[id=%s, %p], with instance[id=%s, %p], "
      "idPrefix: %s",
      json[K_ID].dump().c_str(),
      &json,
      instanceJson[K_ID].dump().c_str(),
      &instanceJson,
      idPrefix.c_str());

    if (json.contains(K_ALPHA_MASK_BY))
    {
      auto& maskBy = json[K_ALPHA_MASK_BY];
      if (maskBy.is_array())
      {
        for (auto& item : maskBy)
        {
          if (item.contains(K_ID))
          {
            auto id = item[K_ID].get<std::string>();
            auto uniqueId = idPrefix + id;
            if (findChildObjectInTree(instanceJson, { uniqueId }, nullptr))
            {
              DEBUG(
                "ExpandSymbol::makeMaskIdUnique: json node[id=%s]: alphaMaskBy, id: %s -> %s",
                json[K_ID].dump().c_str(),
                id.c_str(),
                uniqueId.c_str());
              item[K_ID] = uniqueId;
            }
          }
        }
      }
    }

    if (json.contains(K_OUTLINE_MASK_BY))
    {
      auto& maskBy = json[K_OUTLINE_MASK_BY];
      if (maskBy.is_array())
      {
        for (auto& item : maskBy)
        {
          auto id = item.get<std::string>();
          auto uniqueId = idPrefix + id;
          if (findChildObjectInTree(instanceJson, { uniqueId }, nullptr))
          {
            DEBUG(
              "ExpandSymbol::makeMaskIdUnique: json node[id=%s]: outlineMaskBy, id: %s -> %s",
              json[K_ID].dump().c_str(),
              id.c_str(),
              uniqueId.c_str());
            item = uniqueId;
          }
        }
      }
    }

    if (json.contains(K_CHILD_OBJECTS))
    {
      makeMaskIdUnique(json[K_CHILD_OBJECTS], instanceJson, idPrefix);
    }
  }
  else if (json.is_array())
  {
    for (auto& el : json.items())
    {
      makeMaskIdUnique(el.value(), instanceJson, idPrefix);
    }
  }
}

nlohmann::json* ExpandSymbol::findChildObject(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack,
  const nlohmann::json&           overrideItem,
  std::vector<std::string>&       outChildInstanceIdStack)
{
  auto        instanceMasterId = instance[K_MASTER_ID];
  std::string masterOverrideKey;
  if (m_masters.find(instanceMasterId) != m_masters.end())
  {
    auto& masterJson = m_masters[instanceMasterId];
    masterOverrideKey = masterJson.value(K_OVERRIDE_KEY, K_EMPTY_STRING);
  }

  auto& objectIdPaths = overrideItem[K_OBJECT_ID];
  if (!objectIdPaths.is_array() || objectIdPaths.empty())
  {
    return nullptr;
  }
  else if (
    objectIdPaths.size() == 1 &&
    (objectIdPaths[0] == instanceMasterId || objectIdPaths[0] == masterOverrideKey))
  {
    outChildInstanceIdStack = instanceIdStack;
    return &instance;
  }
  else
  {
    outChildInstanceIdStack = instanceIdStack;
    std::vector<std::string> keyStack;
    for (auto& keyJson : objectIdPaths) // overrideKey or id
    {
      keyStack.push_back(keyJson);
    }
    return findChildObjectInTree(instance[K_CHILD_OBJECTS], keyStack, &outChildInstanceIdStack);
  }
}

bool ExpandSymbol::applyReferenceOverride(
  nlohmann::json&       destObjectJson,
  const std::string&    name,
  const nlohmann::json& value)
{
  if (name == K_STYLE && value.is_string())
  {
    std::string s = value;
    if (s.rfind(K_PREFIX, 0) == 0)
    {
      auto id = s.substr(std::strlen(K_PREFIX));

      if (m_designJson.contains(K_REFERENCES))
      {
        auto& items = m_designJson[K_REFERENCES];
        if (items.is_array())
        {
          for (auto& srcReferenence : items)
          {
            if (srcReferenence[K_ID] == id)
            {
              if (srcReferenence.contains(K_STYLE))
              {
                destObjectJson[K_STYLE] = srcReferenence[K_STYLE];
              }
              if (srcReferenence.contains(K_CONTEXT_SETTINGS))
              {
                destObjectJson[K_CONTEXT_SETTINGS] = srcReferenence[K_CONTEXT_SETTINGS];
              }
              if (srcReferenence.contains(K_FONT_ATTR))
              {
                destObjectJson[K_ATTR] = nlohmann::json::array({ srcReferenence[K_FONT_ATTR] });
              }

              return true;
            }
          }

          WARN("ExpandSymbol::applyReferenceOverride, reference not found, return true to NOT "
               "REPLACE with bad value");
          return true;
        }
      }
    }
  }

  return false;
}

void ExpandSymbol::resizeSubtree(nlohmann::json& subtreeJson, const nlohmann::json& newBoundsJson)
{
  DEBUG(
    "#ExpandSymbol::resizeSubtree: node [id=%s, ptr=%p], value=%s",
    subtreeJson[K_ID].get<std::string>().c_str(),
    &subtreeJson,
    newBoundsJson.dump().c_str());
  if (isLayoutNode(subtreeJson))
  {
    Rect newBounds = newBoundsJson;
    layoutSubtree(subtreeJson[K_ID], newBounds.size, false);
  }
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

void ExpandSymbol::layoutInstance(nlohmann::json& instance, const Size& instanceSize)
{
  std::string nodeId = instance[K_ID];
  auto        node = m_layout->layoutTree()->findDescendantNodeById(nodeId);
  if (!node)
  {
    DEBUG("ExpandSymbol::layoutInstance, instance node not found, id: %s ", nodeId.c_str());
#ifdef DUMP_TREE
    m_layout->layoutTree()->dump();
#endif

    return;
  }

  DEBUG(
    "ExpandSymbol::layoutInstance, instanceId: %s, %s, with size: %f, %f",
    instance[K_ID].get<std::string>().c_str(),
    instance[K_BOUNDS].dump().c_str(),
    instanceSize.width,
    instanceSize.height);

  m_layout->rebuildSubtree(node); // force rebuild subtree with updated rules

  // layout first, update instance size with rule size
  node->setNeedLayout();
  node->layoutIfNeeded();

  // layout with new size
  layoutSubtree(nodeId, instanceSize, true);
}

void ExpandSymbol::overrideLayoutRuleSize(
  const nlohmann::json& instanceId,
  const Size&           instanceSize)
{
  DEBUG(
    "ExpandSymbol::overrideLayoutRuleSize, instanceId: %s, size: %f, %f",
    instanceId.dump().c_str(),
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

void ExpandSymbol::layoutSubtree(
  const nlohmann::json& subtreeNodeId,
  Size                  size,
  bool                  preservingOrigin)
{
  m_layout->resizeNodeThenLayout(subtreeNodeId, size, preservingOrigin);
  overrideLayoutRuleSize(subtreeNodeId, size);
}

void ExpandSymbol::layoutDirtyNodes(nlohmann::json& rootTreeJson)
{
  if (m_tmpDirtyNodeIds.empty())
  {
    return;
  }

  m_layout->layoutNodes(m_tmpDirtyNodeIds);

  m_tmpDirtyNodeIds.clear();
}

nlohmann::json* ExpandSymbol::findOutLayoutObject(
  nlohmann::json&                 instance,
  const std::vector<std::string>& instanceIdStack,
  const nlohmann::json&           overrideItem)
{
  std::vector<std::string> _;
  auto childObject = findChildObject(instance, instanceIdStack, overrideItem, _);
  if (!childObject || !childObject->is_object())
  {
    return nullptr;
  }

  return findOutLayoutObjectById((*childObject)[K_ID]);
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

void ExpandSymbol::deleteLeafElement(nlohmann::json& json, const std::string& key)
{
  if (key == "*")
  {
    while (true)
    {
      auto it = json.begin();
      if (it == json.end())
      {
        break;
      }

      json.erase(it.key());
    }
  }
  else if (json.is_array())
  {
    auto path = nlohmann::json::json_pointer{ "/" + key };
    if (json.contains(path))
    {
      auto index = std::stoul(key);
      json.erase(index);
    }
    else
    {
      DEBUG("invalid array index, %s", key.c_str());
    }
  }
  else
  {
    json.erase(key);
  }
}

void ExpandSymbol::makeNodeKeysUnique(nlohmann::json& json, const std::string& idPrefix)
{
  if (json.contains(K_ID))
  {
    auto objectId = json[K_ID].get<std::string>();
    auto newObjectId = idPrefix + objectId;
    DEBUG(
      "ExpandSymbol::makeNodeKeysUnique: object id: %s -> %s",
      objectId.c_str(),
      newObjectId.c_str());
    json[K_ID] = newObjectId;
  }

  if (json.contains(K_OVERRIDE_KEY))
  {
    auto objectKey = json[K_OVERRIDE_KEY].get<std::string>();
    auto newObjectKey = idPrefix + objectKey;
    DEBUG(
      "ExpandSymbol::makeNodeKeysUnique: object key: %s -> %s",
      objectKey.c_str(),
      newObjectKey.c_str());
    json[K_OVERRIDE_KEY] = newObjectKey;
  }
}

bool ExpandSymbol::hasOriginalLayoutRule(const std::string& id)
{
  return m_layoutRules.find(id) != m_layoutRules.end();
}

bool ExpandSymbol::hasRuntimeLayoutRule(const nlohmann::json& id)
{
  return m_layoutRulesCache->find(id) != m_layoutRulesCache->end();
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

void ExpandSymbol::resetInstanceInfo(nlohmann::json& instance)
{
  instance.erase(K_OVERRIDE_VALUES);
  instance.erase(K_VARIABLE_ASSIGNMENTS);
  if (instance.contains(K_CHILD_OBJECTS))
  {
    // Keep own layout rule; Remove children layout rule only;
    removeInvalidLayoutRule(instance[K_CHILD_OBJECTS]);

    instance.erase(K_CHILD_OBJECTS);
    m_layout->rebuildSubtreeById(instance[K_ID]); // remove all children
  }

  // restore to symbolInstance to expand again
  instance[K_CLASS] = K_SYMBOL_INSTANCE;
}