#include <Core/FontManager.h>
#include <core/SkFont.h>

#include "Core/Node.h"
#include "SkiaImpl/VSkFontMgr.h"

#define VGG_USE_EMBBED_FONT 1

#ifdef VGG_USE_EMBBED_FONT
#include "Resources/FiraSans.hpp"
#include "zip.h"

namespace VGG
{
inline std::vector<char> readFromZip(const char* zip, size_t length, const char* entry)
{
  std::vector<char> data;
  auto zipStream = zip_stream_open(zip, length, 0, 'r');
  if (zipStream)
  {
    if (0 == zip_entry_open(zipStream, entry))
    {
      auto size = zip_entry_size(zipStream);
      data.resize(size);
      zip_entry_noallocread(zipStream, data.data(), data.size());
      zip_entry_close(zipStream);
    }
  }
  zip_close(zipStream);
  return data;
}
}; // namespace VGG

#endif
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
      bool atLeastOne = false;
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
          if (createFontManager(p.first, *dirIt, fallbackFonts))
          {
            atLeastOne = true;
          }
        }
      }
      if (auto it = font->find("defaultFontCollection");
          atLeastOne && it != font->end() && it->is_string())
      {
        if (auto dft = coll.find(*it); dft != coll.end())
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
  }
  if (d_ptr->defaultFontManagerKey.empty())
  {
    createFallbackFontMananger();
  }
}

void FontManager::createFallbackFontMananger()
{
  const string key = "default";
  sk_sp<SkFontMgrVGG> vggFontMgr = VGGFontDirectory(nullptr);
  std::vector<std::string> fallbackFonts;
#ifdef VGG_USE_EMBBED_FONT
  auto data = readFromZip((const char*)VGG::layer::resources::FiraSans::FIRA_SANS_ZIP,
                          VGG::layer::resources::FiraSans::FiraSans::DATA_LENGTH,
                          "FiraSans.ttf");
  vggFontMgr->addFont((uint8_t*)data.data(),
                      data.size(),
                      layer::resources::FiraSans::FONT_NAME,
                      true);
  vggFontMgr->saveFontInfo(key + "_fontname.txt");
  fallbackFonts.push_back(layer::resources::FiraSans::FONT_NAME);
#endif
  d_ptr->fontMgrs.insert({ key, std::move(FontMgrData(vggFontMgr, fallbackFonts)) });
  d_ptr->defaultFontManagerKey = key;
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
    if (vggFontMgr)
    {
#ifdef VGG_USE_EMBBED_FONT
      auto data = readFromZip((const char*)VGG::layer::resources::FiraSans::FIRA_SANS_ZIP,
                              VGG::layer::resources::FiraSans::FiraSans::DATA_LENGTH,
                              "FiraSans.ttf");
      vggFontMgr->addFont((uint8_t*)data.data(),
                          data.size(),
                          layer::resources::FiraSans::FONT_NAME,
                          true);
      fallbackFonts.push_back(layer::resources::FiraSans::FONT_NAME);
#endif
      vggFontMgr->saveFontInfo(key + "_fontname.txt");
      d_ptr->fontMgrs.insert({ key, std::move(FontMgrData(vggFontMgr, std::move(fallbackFonts))) });
      return vggFontMgr.get();
    }
    return nullptr;
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
