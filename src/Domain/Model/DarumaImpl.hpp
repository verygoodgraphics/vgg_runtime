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

#include <nlohmann/json.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace VGG::Model::Detail
{

struct Breakpoint
{
  int                                          minWidth{ 0 };
  std::unordered_map<std::string, std::string> themeFrameIdMap; // key: theme; value: frameId
};

// NOLINTBEGIN
void from_json(const nlohmann::json& j, Breakpoint& b);
void to_json(nlohmann::json& j, const Breakpoint& b);
// NOLINTEND

struct Settings
{
  std::string              launchFrameId;
  std::string              currentTheme;
  std::vector<std::string> themes;
  std::vector<Breakpoint>  breakpoints;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
    Settings,
    launchFrameId,
    currentTheme,
    themes,
    breakpoints)
};

class DarumaImpl
{
  Settings m_settings;

public:
  const Settings& settings() const;
  void            setSettings(const Settings& settings);

  const std::string currentTheme() const;
  bool              setCurrentTheme(const std::string& name);

  void setLaunchFrameById(const std::string& id);

  const std::string frameIdForWidth(double width) const;

private:
  void              sortBreakpoints();
  bool              isValidTheme(const std::string& name) const;
  const std::string frameIdForCurrentTheme(const Breakpoint& breakpoint) const;
};

} // namespace VGG::Model::Detail