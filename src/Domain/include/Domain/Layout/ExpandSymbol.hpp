#pragma once

#include "Rect.hpp"

#include "nlohmann/json.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace VGG
{
namespace Layout
{
constexpr auto K_SEPARATOR = "__";

class ExpandSymbol
{
  const nlohmann::json m_designJson;
  std::unordered_map<std::string, nlohmann::json> m_masters;

public:
  ExpandSymbol(const nlohmann::json& designJson)
    : m_designJson(designJson)
  {
  }

  nlohmann::json operator()();

private:
  void collectMaster(const nlohmann::json& json);
  void expandInstance(nlohmann::json& json,
                      std::vector<std::string>& instanceIdStack,
                      bool again = false);
  void scaleFromMaster(nlohmann::json& instance, nlohmann::json& master);

  void normalizeChildrenGeometry(nlohmann::json& json, const Size containerSize);
  void normalizePoint(nlohmann::json& json, const char* key, const Size& containerSize);
  void recalculateIntanceChildrenGeometry(nlohmann::json& json, Size containerSize);
  void recalculatePoint(nlohmann::json& json, const char* key, const Size& containerSize);

  void applyOverrides(nlohmann::json& instance, const std::vector<std::string>& instanceIdStack);
  void processMasterIdOverrides(nlohmann::json& instance,
                                const std::vector<std::string>& instanceIdStack);
  void processOtherOverrides(nlohmann::json& instance,
                             const std::vector<std::string>& instanceIdStack);
  void applyOverridesDetail(nlohmann::json& json,
                            std::stack<std::string> reversedPath,
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
};
} // namespace Layout

} // namespace VGG
