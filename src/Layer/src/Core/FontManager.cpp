#include <Core/FontManager.h>

#include "Core/Node.hpp"
#include "SkiaImpl/VSkFontMgr.h"
namespace VGG
{
using namespace std;

struct FontMgrData
{
  sk_sp<SkFontMgrVGG> FontMgr;
  std::vector<std::string> FallbackFonts;
  FontMgrData(sk_sp<SkFontMgrVGG> fontMgr, std::vector<std::string> fallbackFonts)
    : FontMgr(std::move(fontMgr))
    , FallbackFonts(std::move(fallbackFonts))
  {
  }
};
class FontManager__pImpl
{
  VGG_DECL_API(FontManager);

public:
  FontManager__pImpl(FontManager* api)
    : q_ptr(api)
  {
  }

  std::unordered_map<std::string, FontMgrData> fontMgrs;
  std::string defaultFontManagerKey;
};

FontManager::FontManager()
  : d_ptr(new FontManager__pImpl(this))
{
  auto& cfg = Config::globalConfig();
  if (auto font = cfg.find("fonts"); font != cfg.end())
  {
    if (auto it = font->find("fontCollections"); it != font->end() && it->is_object())
    {
      std::unordered_map<std::string, nlohmann::json> coll = *it;
      for (const auto& p : coll)
      {
        if (auto dirIt = p.second.find("dir"); dirIt != p.second.end() && dirIt->is_string())
        {
          std::vector<std::string> fallbackFonts;
          if (auto fallbackIt = p.second.find("fallbackFonts");
              fallbackIt != p.second.end() && fallbackIt->is_array())
          {
            fallbackFonts = *fallbackIt;
          }
          createFontManager(p.first, *dirIt, fallbackFonts);
        }
      }
    }
    if (auto it = font->find("defaultFontCollection"); it != font->end() && it->is_string())
    {
      d_ptr->defaultFontManagerKey = *it;
    }
    else
    {
      if (auto it = d_ptr->fontMgrs.begin(); it != d_ptr->fontMgrs.end())
      {
        d_ptr->defaultFontManagerKey = it->first;
      }
    }
  }
}

SkFontMgrVGG* FontManager::createFontManager(const std::string& key,
                                             const fs::path& fontDir,
                                             std::vector<std::string> fallbackFonts)
{
  if (auto it = d_ptr->fontMgrs.find(key); it != d_ptr->fontMgrs.end())
  {
    return it->second.FontMgr.get();
  }
  else
  {
    sk_sp<SkFontMgrVGG> vggFontMgr = VGGFontDirectory(fontDir.string().c_str());
    vggFontMgr->saveFontInfo(key + "_fontname.txt");
    d_ptr->fontMgrs.insert({ key, std::move(FontMgrData(vggFontMgr, std::move(fallbackFonts))) });
    return vggFontMgr.get();
  }
}

SkFontMgrVGG* FontManager::getFontManager(const std::string& key) const
{
  if (auto it = d_ptr->fontMgrs.find(key); it != d_ptr->fontMgrs.end())
  {
    return it->second.FontMgr.get();
  }
  return nullptr;
}

std::vector<std::string> FontManager::getFallbackFonts(const std::string& key) const
{
  if (auto it = d_ptr->fontMgrs.find(key); it != d_ptr->fontMgrs.end())
  {
    return it->second.FallbackFonts;
  }
  return {};
}

void FontManager::setDefaultFontManager(const std::string& key)
{
  d_ptr->defaultFontManagerKey = key;
}

SkFontMgrVGG* FontManager::getDefaultFontManager() const
{
  return getFontManager(d_ptr->defaultFontManagerKey);
}
std::vector<std::string> FontManager::getDefaultFallbackFonts() const
{
  return getFallbackFonts(d_ptr->defaultFontManagerKey);
}

FontManager::~FontManager() = default;

} // namespace VGG
