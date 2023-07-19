#include <Core/FontManager.h>
#include <core/SkRefCnt.h>
namespace VGG
{
using namespace std;

FontManager::FontManager()
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
          createOrGetFontCollection(p.first, *dirIt, fallbackFonts);
        }
      }
    }
    if (auto it = cfg.find("defaultFontCollection"); it != cfg.end() && it->is_string())
    {
      defaultFontCollectionKey = *it;
      m_defaultFontCollection = fontCollection(defaultFontCollectionKey);
    }
    else
    {
      if (auto it = fontResourceCache.begin(); it != fontResourceCache.end())
      {
        defaultFontCollectionKey = it->first;
        m_defaultFontCollection = it->second;
      }
    }
  }
}
sk_sp<VGGFontCollection> FontManager::createOrGetFontCollection(
  const std::string& key,
  const fs::path& fontDirs,
  const std::vector<std::string>& fallbackFonts)
{
  sk_sp<VGGFontCollection> fontCollection;
  if (auto it = fontResourceCache.find(key); it != fontResourceCache.end())
  {
    return it->second;
  }
  else
  {
    fontCollection = sk_make_sp<VGGFontCollection>(fontDirs, fallbackFonts);
    fontResourceCache[key] = fontCollection;
    return fontCollection;
  }
}

sk_sp<VGGFontCollection> FontManager::fontCollection(const std::string& key)
{
  sk_sp<VGGFontCollection> fontCollection;
  if (auto it = fontResourceCache.find(key); it != fontResourceCache.end())
  {
    fontCollection = it->second;
  }
  return fontCollection;
}

} // namespace VGG
