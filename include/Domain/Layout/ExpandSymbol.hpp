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

class ExpandSymbol
{
  const nlohmann::json m_designJson;
  const nlohmann::json m_layoutJson;
  nlohmann::json m_outLayoutJson;
  std::unordered_map<std::string, nlohmann::json> m_masters;
  std::unordered_map<std::string, nlohmann::json> m_layoutRules;

public:
  ExpandSymbol(const nlohmann::json& designJson,
               const nlohmann::json& layoutJson = nlohmann::json())
    : m_designJson(designJson)
    , m_layoutJson(layoutJson)
  {
  }

  nlohmann::json operator()();
  std::pair<nlohmann::json, nlohmann::json> run(); // 0: design.json; 1: layout.json

private:
  void collectMaster(const nlohmann::json& json);
  void collectLayoutRules(const nlohmann::json& json);

  void expandInstance(nlohmann::json& json,
                      std::vector<std::string>& instanceIdStack,
                      bool again = false);
  void scaleFromMaster(nlohmann::json& instance, nlohmann::json& master);

  void normalizeChildrenGeometry(nlohmann::json& json, const Size containerSize);
  void normalizePoint(nlohmann::json& json, const char* key, const Size& containerSize);
  void recalculateIntanceChildrenGeometry(nlohmann::json& json, Size containerSize);
  void recalculatePoint(nlohmann::json& json, const char* key, const Size& containerSize);
  void scaleTree(nlohmann::json& rootJson, const nlohmann::json& newBoundsJson);
  void scaleTree(nlohmann::json& rootJson, float xScaleFactor, float yScaleFactor);
  void scaleContour(nlohmann::json& nodeJson, float xScaleFactor, float yScaleFactor);
  void scalePoint(nlohmann::json& json, const char* key, float xScaleFactor, float yScaleFactor);

  void processMasterIdOverrides(nlohmann::json& instance,
                                const std::vector<std::string>& instanceIdStack);
  void processBoundsOverrides(nlohmann::json& instance,
                              const std::vector<std::string>& instanceIdStack);
  void processOtherOverrides(nlohmann::json& instance,
                             const std::vector<std::string>& instanceIdStack);
  void applyOverridesDetail(nlohmann::json& json,
                            std::stack<std::string> reversedPath,
                            const nlohmann::json& value);
  bool applyReferenceOverride(nlohmann::json& objectJson,
                              const std::string& name,
                              const nlohmann::json& value);

  nlohmann::json* findChildObject(nlohmann::json& instance,
                                  const std::vector<std::string>& instanceIdStack,
                                  std::vector<std::string>& expandContextInstanceIdStack,
                                  nlohmann::json& overrideItem);
  nlohmann::json* findChildObjectInTree(nlohmann::json& json, const std::string& objectId);

  void makeIdUnique(nlohmann::json& json, const std::string& idPrefix);
  void makeMaskIdUnique(nlohmann::json& json,
                        nlohmann::json& instanceJson,
                        const std::string& idPrefix);
  std::string join(const std::vector<std::string>& instanceIdStack,
                   const std::string& seperator = K_SEPARATOR);

  void copyLayoutRule(const std::string& srcId, const std::string& dstId);
  void removeInvalidLayoutRule(const nlohmann::json& instanceChildren);
};
} // namespace Layout

} // namespace VGG
