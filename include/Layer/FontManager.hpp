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
#include "Layer/Config.hpp"
#include "Utility/HelperMacro.hpp"
#include "Utility/ConfigManager.hpp"

#include <string>
#include <unordered_map>
#include <filesystem>
#include <iostream>

class SkFontMgrVGG;
namespace VGG
{
namespace fs = std::filesystem;
class FontManager__pImpl;
class FontManager final
{
  VGG_DECL_IMPL(FontManager);

public:
  FontManager(const FontManager& s) = delete;
  FontManager& operator=(const FontManager&) = delete;
  FontManager(FontManager&& s) noexcept = delete;
  FontManager&  operator=(FontManager&&) noexcept = delete;
  SkFontMgrVGG* fontManager(const std::string& key) const;
  SkFontMgrVGG* defaultFontManager() const;
  ~FontManager();
  static FontManager& instance()
  {
    static FontManager s_fontManagerInstance;
    return s_fontManagerInstance;
  }

private:
  FontManager();
};

} // namespace VGG
