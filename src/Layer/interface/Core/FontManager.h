#pragma once
#include "Core/Node.h"
#include "Utility/ConfigMananger.h"
#include <core/SkString.h>
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
  FontManager& operator=(FontManager&&) noexcept = delete;
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
