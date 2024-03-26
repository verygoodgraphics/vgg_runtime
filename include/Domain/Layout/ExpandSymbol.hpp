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

#include "Domain/Model/DesignModel.hpp"
#include "Domain/Model/Element.hpp"

#include <nlohmann/json.hpp>

#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace VGG
{
class LayoutNode;
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

  const Model::DesignModel                                    m_designModel;
  const nlohmann::json                                        m_layoutJson;
  std::unordered_map<std::string, nlohmann::json>             m_outLayoutJsonMap; // performance
  RuleMapPtr                                                  m_layoutRulesCache; // performance
  std::unordered_map<std::string, const Model::SymbolMaster*> m_pMasters;
  std::unordered_map<std::string, nlohmann::json>             m_layoutRules;

  std::shared_ptr<VGG::Layout::Layout>         m_layout;
  std::shared_ptr<VGG::Domain::DesignDocument> m_designDocument;

  std::vector<std::string> m_tmpDirtyNodeIds;

public:
  ExpandSymbol(
    const nlohmann::json& designJson,
    const nlohmann::json& layoutJson = nlohmann::json());

  std::pair<std::shared_ptr<VGG::Domain::DesignDocument>, nlohmann::json>
                                            operator()(); // 0: design document tree; 1: layout.json
  std::pair<nlohmann::json, nlohmann::json> run();        // 0: design.json; 1: layout.json

  std::shared_ptr<VGG::Layout::Layout> layout() const;

private:
  enum class EProcessVarRefOption
  {
    ALL,
    ONLY_MASTER,
    NOT_MASTER,
  };

  void collectMasters();
  void collectMasterFromContainer(const Model::Container& conntainer);
  template<typename T>
  void collectMasterFromVariant(const T& variantNode);
  void collectLayoutRules(const nlohmann::json& json);

  void traverseElementNode(
    std::shared_ptr<Domain::Element> element,
    std::vector<std::string>&        instanceIdStack);
  void expandInstanceElement(
    Domain::SymbolInstanceElement& instance,
    std::vector<std::string>&      instanceIdStack,
    bool                           again = false);

  void resizeInstance(
    Domain::SymbolInstanceElement&     instance,
    const Domain::SymbolMasterElement& master);

  void layoutInstance(Domain::SymbolInstanceElement& instance, const Size& instanceSize);
  void overrideLayoutRuleSize(const std::string& instanceId, const Size& instanceSize);

  void layoutSubtree(const std::string& subtreeNodeId, Size size, bool preservingOrigin);
  void layoutSubtree(std::shared_ptr<LayoutNode> subtreeNode, Size size, bool preservingOrigin);
  void layoutDirtyNodes(const std::string& instanceId);

  void processMasterIdOverrides(
    Domain::SymbolInstanceElement&  instance,
    const std::vector<std::string>& instanceIdStack);
  void processVariableAssignmentsOverrides(
    Domain::SymbolInstanceElement&  instance,
    const std::vector<std::string>& instanceIdStack);
  void processVariableRefs(
    std::shared_ptr<Domain::Element> element,   // in instance tree
    std::shared_ptr<Domain::Element> container, // container
    const std::vector<std::string>&  instanceIdStack,
    EProcessVarRefOption             option);
  void processLayoutOverrides(
    Domain::SymbolInstanceElement&  instance,
    const std::vector<std::string>& instanceIdStack);
  void processBoundsOverrides(
    Domain::SymbolInstanceElement&  instance,
    const std::vector<std::string>& instanceIdStack);
  void processOtherOverrides(
    Domain::SymbolInstanceElement&  instance,
    const std::vector<std::string>& instanceIdStack);
  void applyOverrides(nlohmann::json& json, std::string& path, const nlohmann::json& value);
  void applyOverridesDetail(
    nlohmann::json&         json,
    std::stack<std::string> reversedPath,
    const nlohmann::json&   value);
  void applyLeafOverrides(
    nlohmann::json&       json,
    const std::string&    key,
    const nlohmann::json& value);
  bool applyReferenceOverride(
    Domain::Element&      objectJson,
    const std::string&    name,
    const nlohmann::json& value);

  std::shared_ptr<Domain::Element> findChildObject(
    Domain::SymbolInstanceElement&  instance,
    const std::vector<std::string>& instanceIdStack,
    const Model::OverrideValue&     overrideItem,
    std::vector<std::string>&       outChildInstanceIdStack);

  void makeTreeKeysUnique(std::shared_ptr<Domain::Element>& element, const std::string& idPrefix);
  void makeMaskIdUnique(
    std::shared_ptr<Domain::Element>& element,
    Domain::SymbolInstanceElement&    instance,
    const std::string&                idPrefix);
  std::string join(
    const std::vector<std::string>& instanceIdStack,
    const std::string&              seperator = K_SEPARATOR);

  void            mergeLayoutRule(const std::string& srcId, const std::string& dstId);
  void            removeInvalidLayoutRule(const nlohmann::json& instanceChildren);
  void            removeInvalidLayoutRule(const Domain::Element& element, bool keepOwn = false);
  bool            hasOriginalLayoutRule(const std::string& id);
  nlohmann::json* findOutLayoutObjectById(const std::string& id);

  RuleMapPtr     getLayoutRules();
  nlohmann::json generateOutLayoutJson();

  void resetInstanceInfo(Domain::SymbolInstanceElement& instance);
};
} // namespace Layout

} // namespace VGG
