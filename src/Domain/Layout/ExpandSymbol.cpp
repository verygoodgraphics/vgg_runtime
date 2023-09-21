#include "ExpandSymbol.hpp"

#include "Layout.hpp"
#include "ReferenceJsonDocument.hpp"

#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Utility/Log.hpp"

#include <algorithm>
#include <fstream>
#include <numeric>
#include <iostream>

#undef DEBUG
#define DEBUG(msg, ...)

constexpr auto K_PREFIX = "referenced_style_";

namespace nl = nlohmann;
using namespace VGG::Layout;

nlohmann::json ExpandSymbol::operator()()
{
  return std::get<0>(run());
}

std::pair<nlohmann::json, nlohmann::json> ExpandSymbol::run()
{
  collectMaster(m_designJson);
  collectLayoutRules(m_layoutJson);

  auto designJson = m_designJson;
  m_outLayoutJson = m_layoutJson;

  std::vector<std::string> instanceIdStack{};
  expandInstance(designJson, instanceIdStack);

  return { designJson, m_outLayoutJson };
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

        // 2 make instance tree nodes id unique
        // 2.1
        auto newInstanceId = join(instanceIdStack);
        auto idPrefix = newInstanceId + K_SEPARATOR;
        DEBUG("ExpandSymbol: instance id: %s -> %s", instanceId.c_str(), newInstanceId.c_str());
        json[K_ID] = newInstanceId; // self
        copyLayoutRule(masterId, newInstanceId);
        makeIdUnique(json[K_CHILD_OBJECTS], idPrefix); // children
        // 2.2. update mask by: id -> unique id
        makeMaskIdUnique(json[K_CHILD_OBJECTS], json, idPrefix);

        // 3 overrides and scale
        // 3.1 master id overrides
        processMasterIdOverrides(json, instanceIdStack);
        // 3.2: scale or layout before bounds overrides
        resizeInstance(json, masterJson);
        // 3.3 bounds overrides must be processed after scaling
        processBoundsOverrides(json, instanceIdStack);
        // 3.4 other overrides
        processOtherOverrides(json, instanceIdStack);
        if (!again)
        {
          instanceIdStack.pop_back();
        }

        // 4. again, makeMaskIdUnique after override: id -> unique id
        makeMaskIdUnique(json[K_CHILD_OBJECTS], json, idPrefix);

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

void ExpandSymbol::resizeInstance(nlohmann::json& instance, nlohmann::json& master)
{
  auto masterSize = master[K_BOUNDS].get<Rect>().size;
  auto instanceSize = instance[K_BOUNDS].get<Rect>().size;
  if (masterSize == instanceSize)
  {
    return;
  }

  if (hasLayoutRule(master[K_ID]))
  {
    layoutInstance(instance, masterSize, instanceSize);
  }
  else
  {
    scaleInstance(instance, masterSize, instanceSize);
  }
}

void ExpandSymbol::scaleInstance(nlohmann::json& instance,
                                 const Size& masterSize,
                                 const Size& instanceSize)
{
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

    DEBUG("ExpandSymbol::normalizePoint: %s -> [%f,%f]",
          json[key].dump().c_str(),
          point.x,
          point.y);
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

    DEBUG("ExpandSymbol::recalculatePoint: %s -> [%f,%f]",
          json[key].dump().c_str(),
          point.x,
          point.y);
    json[key] = point;
  }
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
    removeInvalidLayoutRule((*childObject));

    // restore to symbolInstance to expand again
    (*childObject)[K_CLASS] = K_SYMBOL_INSTANCE;
    expandInstance(*childObject, expandContextInstanceIdStack, true);
  }
}

void ExpandSymbol::processBoundsOverrides(nlohmann::json& instance,
                                          const std::vector<std::string>& instanceIdStack)
{
  const auto& overrideValues = instance[K_OVERRIDE_VALUES];
  if (!overrideValues.is_array())
  {
    return;
  }

  auto boundsOverrideValues = nlohmann::json::array();
  std::copy_if(overrideValues.begin(),
               overrideValues.end(),
               std::back_inserter(boundsOverrideValues),
               [](const nlohmann::json& item) { return item[K_OVERRIDE_NAME] == K_BOUNDS; });

  // Sorting: Top-down, root first;
  std::stable_sort(boundsOverrideValues.begin(),
                   boundsOverrideValues.end(),
                   [](const nlohmann::json& a, const nlohmann::json& b)
                   {
                     auto& aObjectIdPaths = a[K_OBJECT_ID];
                     auto& bObjectIdPaths = b[K_OBJECT_ID];
                     return aObjectIdPaths.size() < bObjectIdPaths.size();
                   });

  auto instanceId = instance[K_ID];
  for (auto& el : boundsOverrideValues.items())
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
    resizeTree(*childObject, value);
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
    if (name == K_MASTER_ID || name == K_BOUNDS || name.empty())
    {
      continue;
    }

    auto& value = overrideItem[K_OVERRIDE_VALUE];
    if (applyReferenceOverride(*childObject, name, value))
    {
      continue;
    }

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
    std::stack<std::string> pathStack;
    while (!path.empty())
    {
      pathStack.push(path.back());
      path.pop_back();
    }

    applyOverridesDetail((*childObject), pathStack, value);
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
    if (json.is_array())
    {
      auto path = nlohmann::json::json_pointer{ "/" + key };
      if (json.contains(path))
      {
        if (isLastKey)
        {
          json[path] = value;
        }
        else
        {
          applyOverridesDetail(json[path], reversedPath, value);
        }
      }
      else
      {
        DEBUG("invalid array index, %s", key.c_str());
      }
    }
    else
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
      copyLayoutRule(objectId, newObjectId);
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

bool ExpandSymbol::applyReferenceOverride(nlohmann::json& destObjectJson,
                                          const std::string& name,
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

void ExpandSymbol::resizeTree(nlohmann::json& rootJson, const nlohmann::json& newBoundsJson)
{
  if (isLayoutNode(rootJson) && hasLayoutRule(rootJson[K_ID]))
  {
    layoutTree(rootJson, newBoundsJson);
  }
  else
  {
    scaleTree(rootJson, newBoundsJson);
  }
}

void ExpandSymbol::scaleTree(nlohmann::json& rootJson, const nlohmann::json& newBoundsJson)
{
  Rect newBounds = newBoundsJson;

  auto hasBounds = isLayoutNode(rootJson);
  Rect oldBounds, frame;
  Matrix matrix;
  if (!hasBounds)
  {
    return;
  }

  oldBounds = rootJson[K_BOUNDS].get<Rect>();
  if (newBounds == oldBounds)
  {
    return;
  }

  auto newSize = newBounds.size;
  auto oldSize = oldBounds.size;
  auto xScaleFactor = newSize.width / oldSize.width;
  auto yScaleFactor = newSize.height / oldSize.height;

  DEBUG("ExpandSymbol::scaleTree, id=%s, bounds: %s -> %s",
        rootJson[K_ID].dump().c_str(),
        rootJson[K_BOUNDS].dump().c_str(),
        newBoundsJson.dump().c_str());
  rootJson[K_BOUNDS] = newBoundsJson;
  scaleContour(rootJson, xScaleFactor, yScaleFactor);

  for (auto& el : rootJson.items())
  {
    scaleTree(el.value(), xScaleFactor, yScaleFactor);
  }
}

void ExpandSymbol::scaleTree(nlohmann::json& rootJson, float xScaleFactor, float yScaleFactor)
{
  if (!rootJson.is_object() && !rootJson.is_array())
  {
    return;
  }

  auto hasBounds = isLayoutNode(rootJson);
  Rect bounds, frame;
  Matrix matrix;
  if (hasBounds)
  {
    bounds = rootJson[K_BOUNDS].get<Rect>();
    frame = rootJson[K_FRAME].get<Rect>();
    matrix = rootJson[K_MATRIX].get<Matrix>();

    bounds.origin.x *= xScaleFactor;
    bounds.origin.y *= yScaleFactor;
    bounds.size.width *= xScaleFactor;
    bounds.size.height *= yScaleFactor;

    frame.origin.x *= xScaleFactor;
    frame.origin.y *= yScaleFactor;
    frame.size.width *= xScaleFactor;
    frame.size.height *= yScaleFactor;

    matrix.tx *= xScaleFactor;
    matrix.ty *= yScaleFactor;

    DEBUG("ExpandSymbol::scaleTree, id=%s", rootJson[K_ID].dump().c_str());
    DEBUG("ExpandSymbol::scaleTree, bounds, %s -> %f, %f, %f, %f",
          rootJson[K_BOUNDS].dump().c_str(),
          bounds.origin.x,
          bounds.origin.y,
          bounds.size.width,
          bounds.size.height);
    DEBUG("ExpandSymbol::scaleTree, frames, %s -> %f, %f, %f, %f",
          rootJson[K_FRAME].dump().c_str(),
          frame.origin.x,
          frame.origin.y,
          frame.size.width,
          frame.size.height);
    DEBUG("ExpandSymbol::scaleTree, matrix, %s -> %f, %f",
          rootJson[K_MATRIX].dump().c_str(),
          matrix.tx,
          matrix.ty);
    to_json(rootJson[K_BOUNDS], bounds);
    to_json(rootJson[K_FRAME], frame);
    to_json(rootJson[K_MATRIX], matrix);

    scaleContour(rootJson, xScaleFactor, yScaleFactor);
  }

  for (auto& el : rootJson.items())
  {
    scaleTree(el.value(), xScaleFactor, yScaleFactor);
  }
}

void ExpandSymbol::scaleContour(nlohmann::json& nodeJson, float xScaleFactor, float yScaleFactor)
{
  if (nodeJson[K_CLASS] != K_PATH)
  {
    return;
  }

  DEBUG("ExpandSymbol::scaleContour, id=%s", nodeJson[K_ID].dump().c_str());

  // ./shape/subshapes/XXX/subGeometry/points/XXX
  auto& subshapes = nodeJson[K_SHAPE][K_SUBSHAPES];
  for (auto i = 0; i < subshapes.size(); ++i)
  {
    auto& subshape = subshapes[i];
    auto& subGeometry = subshape[K_SUBGEOMETRY];
    if (subGeometry[K_CLASS] != K_CONTOUR)
    {
      continue;
    }

    auto& points = subGeometry[K_POINTS];
    for (auto j = 0; j < points.size(); ++j)
    {
      auto& point = points[j];
      scalePoint(point, K_POINT, xScaleFactor, yScaleFactor);
      scalePoint(point, K_CURVE_FROM, xScaleFactor, yScaleFactor);
      scalePoint(point, K_CURVE_TO, xScaleFactor, yScaleFactor);
    }
  }
}

void ExpandSymbol::scalePoint(nlohmann::json& json,
                              const char* key,
                              float xScaleFactor,
                              float yScaleFactor)
{
  if (json.contains(key))
  {
    auto point = json[key].get<Point>();

    point.x *= xScaleFactor;
    point.y *= yScaleFactor;

    DEBUG("ExpandSymbol::scalePoint, %s -> %f, %f", json[key].dump().c_str(), point.x, point.y);
    json[key] = point;
  }
}

void ExpandSymbol::copyLayoutRule(const std::string& srcId, const std::string& dstId)
{
  if (srcId == dstId)
  {
    return;
  }

  if (!hasLayoutRule(srcId))
  {
    return;
  }

  auto rule = m_layoutRules[srcId];
  rule[K_ID] = dstId;
  m_layoutRules[dstId] = rule;
  m_outLayoutJson[K_OBJ].push_back(std::move(rule));
}

void ExpandSymbol::removeInvalidLayoutRule(const nlohmann::json& json)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.is_object() && json.contains(K_ID))
  {
    auto id = json[K_ID];
    m_layoutRules.erase(id);

    auto& rules = m_outLayoutJson[K_OBJ];
    for (auto i = 0; i < rules[K_OBJ].size(); ++i)
    {
      if (rules[i][K_ID] == id)
      {
        rules.erase(i);
        break;
      }
    }
  }

  for (auto& el : json.items())
  {
    removeInvalidLayoutRule(el.value());
  }
}

void ExpandSymbol::layoutInstance(nlohmann::json& instance,
                                  const Size& masterSize,
                                  const Size& instanceSize)
{
  Layout layout{ JsonDocumentPtr{ new ReferenceJsonDocument{ instance } },
                 JsonDocumentPtr{ new ReferenceJsonDocument{ m_outLayoutJson } },
                 false };
  // layout.layout(instanceSize); // not work, instance size is same as it is
  auto root = layout.layoutTree();
  root->setNeedLayout();
  root->layoutIfNeeded();
}

void ExpandSymbol::layoutTree(nlohmann::json& rootJson, const nlohmann::json& newBoundsJson)
{
  Rect newBounds = newBoundsJson;
  Layout layout{ JsonDocumentPtr{ new ReferenceJsonDocument{ rootJson } },
                 JsonDocumentPtr{ new ReferenceJsonDocument{ m_outLayoutJson } },
                 false };
  layout.layout(newBounds.size);
}
