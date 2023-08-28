#include "ExpandSymbol.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"

#include <algorithm>
#include <fstream>
#include <numeric>
#include <iostream>

#undef DEBUG
#define DEBUG(msg, ...)

namespace nl = nlohmann;
using namespace VGG::Layout;

nlohmann::json ExpandSymbol::operator()()
{
  collectMaster(m_designJson);

  auto result = m_designJson;
  std::vector<std::string> instanceIdStack{};
  expandInstance(result, instanceIdStack);

  return result;
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

void ExpandSymbol::expandInstance(nlohmann::json& json,
                                  std::vector<std::string>& instanceIdStack,
                                  bool again)
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

      auto masterId = json[K_MASTER_ID].get<std::string>();
      if (m_masters.find(masterId) != m_masters.end())
      {
        auto instanceId = json[K_ID].get<std::string>();
        DEBUG("#ExpandSymbol: expand instance[id=%s, ptr=%p] with masterId=%s",
              instanceId.c_str(),
              &json,
              masterId.c_str());
        auto& masterJson = m_masters[masterId];
        json[K_CHILD_OBJECTS] = masterJson[K_CHILD_OBJECTS];
        json[K_STYLE] = masterJson[K_STYLE];

        if (!again)
        {
          instanceIdStack.push_back(instanceId);
        }

        // 1. expand
        expandInstance(json[K_CHILD_OBJECTS], instanceIdStack);

        // 2.1. make instance tree nodes id unique
        auto newInstanceId = join(instanceIdStack);
        auto idPrefix = newInstanceId + K_SEPARATOR;
        DEBUG("ExpandSymbol: instance id: %s -> %s", instanceId.c_str(), newInstanceId.c_str());
        json[K_ID] = newInstanceId;                    // self
        makeIdUnique(json[K_CHILD_OBJECTS], idPrefix); // children
        // 2.2. update mask by: id -> unique id
        makeMaskIdUnique(json[K_CHILD_OBJECTS], json, idPrefix);

        // 3. overrides
        applyOverrides(json, instanceIdStack);

        if (!again)
        {
          instanceIdStack.pop_back();
        }

        // 4. scale
        scaleFromMaster(json, masterJson);

        // 5. make instance node to "symbalMaster" or render will not draw this node
        DEBUG("#ExpandSymbol: make instance[id=%s, ptr=%p] as master, erase masterId",
              instanceId.c_str(),
              &json);
        json[K_CLASS] = K_SYMBOL_MASTER;
        json[K_NAME] = json[K_NAME].get<std::string>() + "; expanded_instance";
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

void ExpandSymbol::scaleFromMaster(nlohmann::json& instance, nlohmann::json& master)
{
  auto masterSize = master[K_BOUNDS].get<Rect>().size;
  auto instanceSize = instance[K_BOUNDS].get<Rect>().size;
  if (masterSize == instanceSize)
  {
    return;
  }

  normalizeChildrenGeometry(instance[K_CHILD_OBJECTS], masterSize);
  recalculateIntanceChildrenGeometry(instance[K_CHILD_OBJECTS], instanceSize);
}

void ExpandSymbol::normalizeChildrenGeometry(nlohmann::json& json, const Size containerSize)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }
  auto myContainerSize = containerSize;
  auto childContainerSize = containerSize;

  auto hasBounds = isLayoutNode(json);
  Rect bounds, frame;
  Matrix matrix;
  if (hasBounds)
  {
    bounds = json[K_BOUNDS].get<Rect>();
    frame = json[K_FRAME].get<Rect>();
    matrix = json[K_MATRIX].get<Matrix>();

    childContainerSize = bounds.size;
  }

  // bottom up; leaf first
  for (auto& el : json.items())
  {
    normalizeChildrenGeometry(el.value(), childContainerSize);
  }

  // root last, normalize self
  if (hasBounds)
  {
    Rect normalizedBounds = {
      { bounds.origin.x / myContainerSize.width, bounds.origin.y / myContainerSize.height },
      { bounds.size.width / myContainerSize.width, bounds.size.height / myContainerSize.height }
    };
    Rect normalizedFrame = {
      { frame.origin.x / myContainerSize.width, frame.origin.y / myContainerSize.height },
      { frame.size.width / myContainerSize.width, frame.size.height / myContainerSize.height }
    };
    Matrix normalizedMatrix = matrix;
    normalizedMatrix.tx = matrix.tx / myContainerSize.width;
    normalizedMatrix.ty = matrix.ty / myContainerSize.height;

    to_json(json[K_BOUNDS], normalizedBounds);
    to_json(json[K_FRAME], normalizedFrame);
    to_json(json[K_MATRIX], normalizedMatrix);
  }
  if (isPointAttrNode(json))
  {
    normalizePoint(json, K_POINT, myContainerSize);
    normalizePoint(json, K_CURVE_FROM, myContainerSize);
    normalizePoint(json, K_CURVE_TO, myContainerSize);
  }
}

void ExpandSymbol::normalizePoint(nlohmann::json& json, const char* key, const Size& containerSize)
{
  if (json.contains(key))
  {
    auto point = json[key].get<Point>();

    point.x /= containerSize.width;
    point.y /= containerSize.height;

    json[key] = point;
  }
}

void ExpandSymbol::recalculateIntanceChildrenGeometry(nlohmann::json& json, Size containerSize)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  // top down, root fist
  auto hasBounds = isLayoutNode(json);
  Rect normalizedBounds, normalizedFrame;
  Matrix normalizedMatrix;
  if (hasBounds)
  {
    normalizedBounds = json[K_BOUNDS].get<Rect>();
    normalizedFrame = json[K_FRAME].get<Rect>();
    normalizedMatrix = json[K_MATRIX].get<Matrix>();

    Rect bounds = { { normalizedBounds.origin.x * containerSize.width,
                      normalizedBounds.origin.y * containerSize.height },
                    { normalizedBounds.size.width * containerSize.width,
                      normalizedBounds.size.height * containerSize.height } };
    Rect frame = { { normalizedFrame.origin.x * containerSize.width,
                     normalizedFrame.origin.y * containerSize.height },
                   { normalizedFrame.size.width * containerSize.width,
                     normalizedFrame.size.height * containerSize.height } };
    auto matrix = normalizedMatrix;
    matrix.tx = normalizedMatrix.tx * containerSize.width;
    matrix.ty = normalizedMatrix.ty * containerSize.height;

    to_json(json[K_BOUNDS], bounds);
    to_json(json[K_FRAME], frame);
    to_json(json[K_MATRIX], matrix);

    containerSize = bounds.size;
  }
  if (isPointAttrNode(json))
  {
    recalculatePoint(json, K_POINT, containerSize);
    recalculatePoint(json, K_CURVE_FROM, containerSize);
    recalculatePoint(json, K_CURVE_TO, containerSize);
  }

  // leaf last
  for (auto& el : json.items())
  {
    recalculateIntanceChildrenGeometry(el.value(), containerSize);
  }
}

void ExpandSymbol::recalculatePoint(nlohmann::json& json,
                                    const char* key,
                                    const Size& containerSize)
{
  if (json.contains(key))
  {
    auto point = json[key].get<Point>();

    point.x *= containerSize.width;
    point.y *= containerSize.height;

    json[key] = point;
  }
}

void ExpandSymbol::applyOverrides(nlohmann::json& instance,
                                  const std::vector<std::string>& instanceIdStack)
{
  processMasterIdOverrides(instance, instanceIdStack);
  processOtherOverrides(instance, instanceIdStack);
}

void ExpandSymbol::processMasterIdOverrides(nlohmann::json& instance,
                                            const std::vector<std::string>& instanceIdStack)
{
  const auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto masterIdOverrideValues = nlohmann::json::array();
  std::copy_if(overrideValues.begin(),
               overrideValues.end(),
               std::back_inserter(masterIdOverrideValues),
               [](const nlohmann::json& item) { return item[K_OVERRIDE_NAME] == K_MASTER_ID; });

  // Sorting: Top-down, root first;
  std::stable_sort(masterIdOverrideValues.begin(),
                   masterIdOverrideValues.end(),
                   [](const nlohmann::json& a, const nlohmann::json& b)
                   {
                     auto& aObjectIdPaths = a[K_OBJECT_ID];
                     auto& bObjectIdPaths = b[K_OBJECT_ID];
                     return aObjectIdPaths.size() < bObjectIdPaths.size();
                   });

  auto instanceId = instance[K_ID];
  for (auto& el : masterIdOverrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }

    auto expandContextInstanceIdStack = instanceIdStack;
    nl::json* childObject =
      findChildObject(instance, instanceIdStack, expandContextInstanceIdStack, overrideItem);
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
    (*childObject).erase(K_OVERRIDE_VALUES);

    // restore to symbolInstance to expand again
    (*childObject)[K_CLASS] = K_SYMBOL_INSTANCE;
    expandInstance(*childObject, expandContextInstanceIdStack, true);
  }
}

void ExpandSymbol::processOtherOverrides(nlohmann::json& instance,
                                         const std::vector<std::string>& instanceIdStack)
{
  auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto instanceId = instance[K_ID];
  auto instanceMasterId = instance[K_MASTER_ID];
  for (auto& el : overrideValues.items())
  {
    auto& overrideItem = el.value();
    if (!overrideItem.is_object() || (overrideItem[K_CLASS] != K_OVERRIDE_CLASS))
    {
      continue;
    }

    auto expandContextInstanceIdStack = instanceIdStack;
    nl::json* childObject =
      findChildObject(instance, instanceIdStack, expandContextInstanceIdStack, overrideItem);
    if (!childObject || !childObject->is_object())
    {
      continue;
    }

    std::string name = overrideItem[K_OVERRIDE_NAME];
    auto value = overrideItem[K_OVERRIDE_VALUE];
    if (name != K_MASTER_ID && !name.empty()) // other overrides
    {
      // make name to json pointer string
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
      DEBUG("#ExpandSymbol: override object[id=%s, ptr=%p], path=%s, value=%s",
            (*childObject)[K_ID].get<std::string>().c_str(),
            childObject,
            path.to_string().c_str(),
            value.dump().c_str());
      if (name.find("*") == std::string::npos) // no * in path
      {
        (*childObject)[path] = value;
      }
      else // path has *
      {
        std::stack<std::string> pathStack;
        while (!path.empty())
        {
          pathStack.push(path.back());
          path.pop_back();
        }

        applyOverridesDetail((*childObject), pathStack, value);
      }
    }
  }
}

void ExpandSymbol::applyOverridesDetail(nlohmann::json& json,
                                        std::stack<std::string> reversedPath,
                                        const nlohmann::json& value)
{
  ASSERT(!reversedPath.empty());

  auto key = reversedPath.top();
  reversedPath.pop();
  auto isLastKey = reversedPath.empty();

  if (key != "*")
  {
    if (isLastKey)
    {
      json[key] = value;
    }
    else
    {
      applyOverridesDetail(json[key], reversedPath, value);
    }
  }
  else
  {
    for (auto& el : json.items())
    {
      if (isLastKey)
      {
        el.value() = value;
      }
      else
      {
        applyOverridesDetail(el.value(), reversedPath, value);
      }
    }
  }
}

nlohmann::json* ExpandSymbol::findChildObjectInTree(nlohmann::json& json,
                                                    const std::string& objectId)
{
  if (!json.is_object() && !json.is_array())
  {
    return nullptr;
  }

  if (json.is_object())
  {
    if (json.contains(K_ID) && json[K_ID] == objectId)
    {
      return &json;
    }
  }

  for (auto& el : json.items())
  {
    auto ret = findChildObjectInTree(el.value(), objectId);
    if (ret)
    {
      return ret;
    }
  }

  return nullptr;
}

void ExpandSymbol::makeIdUnique(nlohmann::json& json, const std::string& idPrefix)
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

    if (json.contains(K_ID))
    {
      auto objectId = json[K_ID].get<std::string>();
      auto newObjectId = idPrefix + objectId;
      DEBUG("ExpandSymbol::makeIdUnique: object id: %s -> %s",
            objectId.c_str(),
            newObjectId.c_str());
      json[K_ID] = newObjectId;
    }
  }

  for (auto& el : json.items())
  {
    makeIdUnique(el.value(), idPrefix);
  }
}
std::string ExpandSymbol::join(const std::vector<std::string>& instanceIdStack,
                               const std::string& separator)
{
  return std::accumulate(std::next(instanceIdStack.begin()),
                         instanceIdStack.end(),
                         instanceIdStack[0], // start with first element
                         [&](const std::string& a, const std::string& b)
                         { return a + separator + b; });
}

void ExpandSymbol::makeMaskIdUnique(nlohmann::json& json,
                                    nlohmann::json& instanceJson,
                                    const std::string& idPrefix)
{
  if (json.is_object())
  {
    if (!isLayoutNode(json))
    {
      return;
    }
    DEBUG("ExpandSymbol::makeMaskIdUnique: visit node[id=%s, %p], with instance[id=%s, %p], "
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
            if (findChildObjectInTree(instanceJson, uniqueId))
            {
              DEBUG("ExpandSymbol::makeMaskIdUnique: json node[id=%s]: alphaMaskBy, id: %s -> %s",
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
          if (findChildObjectInTree(instanceJson, uniqueId))
          {
            DEBUG("ExpandSymbol::makeMaskIdUnique: json node[id=%s]: outlineMaskBy, id: %s -> %s",
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
  nlohmann::json& instance,
  const std::vector<std::string>& instanceIdStack,
  std::vector<std::string>& expandContextInstanceIdStack,
  nlohmann::json& overrideItem)
{
  auto instanceMasterId = instance[K_MASTER_ID];

  auto& objectIdPaths = overrideItem[K_OBJECT_ID];
  if (!objectIdPaths.is_array() || objectIdPaths.empty())
  {
    return nullptr;
  }
  else if (objectIdPaths.size() == 1 && objectIdPaths[0] == instanceMasterId)
  {
    return &instance;
  }
  else
  {
    auto newInstanceIdStack = instanceIdStack;
    for (auto& idJson : objectIdPaths)
    {
      newInstanceIdStack.push_back(idJson);
    }
    expandContextInstanceIdStack = newInstanceIdStack;
    return findChildObjectInTree(instance[K_CHILD_OBJECTS], join(newInstanceIdStack));
  }
}