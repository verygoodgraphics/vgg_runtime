#pragma once
#include "Core/Node.hpp"
#include <ConfigMananger.h>
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

class FontManager
{
  VGG_DECL_IMPL(FontManager);

public:
  FontManager(const FontManager& s) = delete;
  void operator=(const FontManager&) = delete;
  SkFontMgrVGG* createFontManager(const std::string& key,
                                  const fs::path& fontDir,
                                  std::vector<std::string> fallbackFonts);
  SkFontMgrVGG* getFontManager(const std::string& key) const;
  std::vector<std::string> getFallbackFonts(const std::string& key) const;
  void setDefaultFontManager(const std::string& key);
  SkFontMgrVGG* getDefaultFontManager() const;
  std::vector<std::string> getDefaultFallbackFonts() const;
  ~FontManager();
  static FontManager& instance()
  {
    static FontManager F;
    return F;
  }

private:
  FontManager();
};

} // namespace VGG
