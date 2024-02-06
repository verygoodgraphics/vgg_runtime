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
#include "Utility/HelperMacro.hpp"

#include <memory>
#include <string>
#include <vector>

class SkFontMgrVGG;

namespace VGG::layer
{
class FontManager__pImpl;

class FontManager final
{
  VGG_DECL_IMPL(FontManager);

public:
  FontManager(const FontManager& s) = delete;
  FontManager& operator=(const FontManager&) = delete;
  FontManager(FontManager&& s) noexcept = delete;
  FontManager& operator=(FontManager&&) noexcept = delete;

  /// @brief fuzzy match the given font family name and
  /// return the actual name with a best score
  std::string matchFontName(std::string_view familyName) const;

  const std::vector<std::string>& fallbackFonts() const;

  /// @brief Add a font from memory
  /// defaultName is the name of the font if there is no name field in the font file,
  /// in most cases, this name is ignored
  bool addFontFromMemory(const uint8_t* data, size_t size, const char* defaultName);

  /// @brief Get the singleton instance of FontManager
  static FontManager& getFontMananger()
  {
    static FontManager s_fontManagerInstance;
    return s_fontManagerInstance;
  }

  ~FontManager();

private:
  friend class SkiaFontManagerProxy;
  FontManager();
};

} // namespace VGG::layer
