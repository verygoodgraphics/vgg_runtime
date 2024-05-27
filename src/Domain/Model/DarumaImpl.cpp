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

#include "DarumaImpl.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <nlohmann/json.hpp>

namespace VGG::Model::Detail
{
constexpr auto K_MIN_WIDTH = "minWidth";

void from_json(const nlohmann::json& j, Breakpoint& b)
{
  for (auto& [key, val] : j.items())
  {
    if (key == K_MIN_WIDTH)
    {
      b.minWidth = val.get<int>();
    }
    else
    {
      b.themeFrameIdMap[key] = val.get<std::string>();
    }
  }
}

void to_json(nlohmann::json& j, const Breakpoint& b)
{
  j = b.themeFrameIdMap;
  j[K_MIN_WIDTH] = b.minWidth;
}

const Settings& DarumaImpl::settings() const
{
  return m_settings;
}

void DarumaImpl::setSettings(const Settings& settings)
{
  m_settings = settings;
  sortBreakpoints();
}

const std::string DarumaImpl::currentTheme() const
{
  if (isValidTheme(m_settings.currentTheme))
  {
    return m_settings.currentTheme;
  }

  if (!m_settings.themes.empty())
  {
    return m_settings.themes[0];
  }

  return {};
}

bool DarumaImpl::setCurrentTheme(const std::string& name)
{
  if (isValidTheme(name))
  {
    m_settings.currentTheme = name;
    return true;
  }

  return false;
}

void DarumaImpl::setLaunchFrameById(const std::string& id)
{
  m_settings.launchFrameId = id;
}

const std::string DarumaImpl::frameIdForWidth(double width) const
{
  auto& breakpoints = m_settings.breakpoints;
  if (breakpoints.empty())
  {
    return {};
  }

  auto found = breakpoints.begin();
  for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it)
  {
    if (width < it->minWidth)
    {
      break;
    }
    found = it;
  }

  return frameIdForCurrentTheme(*found);
}

void DarumaImpl::sortBreakpoints()
{
  std::sort(
    m_settings.breakpoints.begin(),
    m_settings.breakpoints.end(),
    [](const Breakpoint& a, const Breakpoint& b) { return a.minWidth < b.minWidth; });
}

bool DarumaImpl::isValidTheme(const std::string& name) const
{
  auto& themes = m_settings.themes;
  return std::find(themes.begin(), themes.end(), name) != themes.end();
}

const std::string DarumaImpl::frameIdForCurrentTheme(const Breakpoint& breakpoint) const
{
  const auto& themeName = currentTheme();
  const auto& ids = breakpoint.themeFrameIdMap;
  if (ids.contains(themeName))
  {
    return ids.at(themeName);
  }
  else
  {
    return {};
  }
}

} // namespace VGG::Model::Detail