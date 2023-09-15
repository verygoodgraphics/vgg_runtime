#pragma once
#include "Core/Node.h"
#include <Utility/interface/ConfigMananger.h>
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

  void createFallbackFontMananger();

public:
  FontManager(const FontManager& s) = delete;
  FontManager& operator=(const FontManager&) = delete;
  FontManager(FontManager&& s) noexcept = delete;
  FontManager& operator=(FontManager&&) noexcept = delete;
  SkFontMgrVGG* createFontManager(const std::string& key,
                                  const fs::path& fontDir,
                                  std::vector<std::string> fallbackFonts);

  SkFontMgrVGG* fontManager(const std::string& key) const;
  std::vector<std::string> fallbackFonts(const std::string& key) const;
  void setCurrentFontManager(const std::string& key);
  SkFontMgrVGG* currentFontManager() const;
  std::vector<std::string> currentFallbackFonts() const;
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
