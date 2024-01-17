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

#include "Rect.hpp"

#include <nlohmann/json.hpp>

#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace VGG
{
namespace Layout
{
constexpr auto K_SEPARATOR = "__";

class Layout;
namespace Internal
{
namespace Rule
{
struct Rule;
}
} // namespace Internal

class ExpandSymbol
{
  using RuleMap = std::unordered_map<std::string, std::shared_ptr<Internal::Rule::Rule>>;
  using RuleMapPtr = std::shared_ptr<RuleMap>;

  const nlohmann::json&                           m_designJson;
  const nlohmann::json&                           m_layoutJson;
  std::unordered_map<std::string, nlohmann::json> m_outLayoutJsonMap; // performance
  RuleMapPtr                                      m_layoutRulesCache; // performance
  std::unordered_map<std::string, nlohmann::json> m_masters;
  std::unordered_map<std::string, nlohmann::json> m_layoutRules;

  std::shared_ptr<VGG::Layout::Layout> m_layout;

  nlohmann::json           m_tmpOutDesignJson;
  std::vector<std::string> m_tmpDirtyNodeIds;

public:
  ExpandSymbol(
    const nlohmann::json& designJson,
    const nlohmann::json& layoutJson = nlohmann::json())
    : m_designJson(designJson)
    , m_layoutJson(layoutJson)
  {
  }

  nlohmann::json                            operator()();
  std::pair<nlohmann::json, nlohmann::json> run(); // 0: design.json; 1: layout.json

private:
  void collectMaster(const nlohmann::json& json);
  void collectLayoutRules(const nlohmann::json& json);

  void expandInstance(
    nlohmann::json&           json,
    std::vector<std::string>& instanceIdStack,
    bool                      again = false);
  void resizeInstance(nlohmann::json& instance, nlohmann::json& master);

  void layoutInstance(nlohmann::json& instance, const Size& instanceSize);
  void overrideLayoutRuleSize(const nlohmann::json& instanceId, const Size& instanceSize);

  void resizeSubtree(nlohmann::json& subtreeJson, const nlohmann::json& newBoundsJson);

  void layoutSubtree(const nlohmann::json& subtreeNodeId, Size size, bool preservingOrigin);
  void layoutDirtyNodes(nlohmann::json& rootTreeJson);

  void processMasterIdOverrides(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack);
  void processVariableAssignmentsOverrides(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack);
  void processVariableRefs(
    nlohmann::json&                 node,     // in instance tree
    nlohmann::json*                 instance, // container
    const std::vector<std::string>& instanceIdStack);
  void processLayoutOverrides(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack);
  void processBoundsOverrides(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack);
  void processOtherOverrides(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack);
  void applyOverrides(nlohmann::json& json, std::string& path, const nlohmann::json& value);
  void applyOverridesDetail(
    nlohmann::json&         json,
    std::stack<std::string> reversedPath,
    const nlohmann::json&   value);
  void applyOverridesDetailToTree(
    nlohmann::json&         json,
    std::stack<std::string> reversedPath,
    const nlohmann::json&   value);
  void applyLeafOverrides(
    nlohmann::json&       json,
    const std::string&    key,
    const nlohmann::json& value);
  void deleteLeafElement(nlohmann::json& json, const std::string& key);
  bool applyReferenceOverride(
    nlohmann::json&       objectJson,
    const std::string&    name,
    const nlohmann::json& value);

  nlohmann::json* findChildObject(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack,
    const nlohmann::json&           overrideItem,
    std::vector<std::string>&       outChildInstanceIdStack);
  nlohmann::json* findChildObjectInTree(
    nlohmann::json&                 json,
    const std::vector<std::string>& keyStack,
    std::vector<std::string>*       outInstanceIdStack);

  void makeTreeKeysUnique(nlohmann::json& json, const std::string& idPrefix);
  void makeNodeKeysUnique(nlohmann::json& json, const std::string& idPrefix);
  void makeMaskIdUnique(
    nlohmann::json&    json,
    nlohmann::json&    instanceJson,
    const std::string& idPrefix);
  std::string join(
    const std::vector<std::string>& instanceIdStack,
    const std::string&              seperator = K_SEPARATOR);

  void            mergeLayoutRule(const std::string& srcId, const std::string& dstId);
  void            removeInvalidLayoutRule(const nlohmann::json& instanceChildren);
  bool            hasOriginalLayoutRule(const std::string& id);
  bool            hasRuntimeLayoutRule(const nlohmann::json& id);
  nlohmann::json* findOutLayoutObject(
    nlohmann::json&                 instance,
    const std::vector<std::string>& instanceIdStack,
    const nlohmann::json&           overrideItem);
  nlohmann::json* findOutLayoutObjectById(const std::string& id);

  RuleMapPtr     getLayoutRules();
  nlohmann::json generateOutLayoutJson();

  void resetInstanceInfo(nlohmann::json& instance);
};
} // namespace Layout

} // namespace VGG
