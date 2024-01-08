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
#include "VSkFontMgr.hpp"
#include "Utility/ConfigManager.hpp"
#include "Utility/Log.hpp"
#include "Layer/FontManager.hpp"

#include <core/SkFont.h>
#include <iterator>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>

#define VGG_USE_EMBBED_FONT 1

#ifdef VGG_USE_EMBBED_FONT
#include "FiraSans.hpp"
#include <zip.h>

namespace
{

std::vector<std::string_view> split(std::string_view s, char seperator)
{
  std::vector<std::string_view> output;
  std::string::size_type        prevPos = 0, pos = 0;
  while ((pos = s.find(seperator, pos)) != std::string::npos)
  {
    std::string_view substring(s.substr(prevPos, pos - prevPos));
    output.push_back(substring);
    prevPos = ++pos;
  }
  output.push_back(s.substr(prevPos, pos - prevPos)); // Last word
  return output;
}

std::vector<char> readFromZip(const char* zip, size_t length, const char* entry)
{
  std::vector<char> data;
  auto              zipStream = zip_stream_open(zip, length, 0, 'r');
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

bool addEmbbedFont(SkFontMgrVGG* mgr)
{
  auto data = readFromZip(
    (const char*)VGG::layer::resources::FiraSans::FIRA_SANS_ZIP,
    VGG::layer::resources::FiraSans::FiraSans::DATA_LENGTH,
    "FiraSans.ttf");
  return mgr
    ->addFont((uint8_t*)data.data(), data.size(), VGG::layer::resources::FiraSans::FONT_NAME, true);
}

}; // namespace

#endif

namespace
{

template<typename T>
std::vector<T> mergeArrayConfig(std::vector<std::string> config1, std::vector<std::string> config2)
{
  std::set<T> set{ std::make_move_iterator(config1.begin()),
                   std::make_move_iterator(config1.end()) };
  set.insert(std::make_move_iterator(config2.begin()), std::make_move_iterator(config2.end()));
  return { std::make_move_iterator(set.begin()), std::make_move_iterator(set.end()) };
}

} // namespace

namespace VGG::layer
{

class FontManager__pImpl
{
  VGG_DECL_API(FontManager);

public:
  FontManager__pImpl(FontManager* api)
    : q_ptr(api)
  {
  }

  std::unordered_map<std::string, sk_sp<SkFontMgrVGG>> fontMgrs;
  SkFontMgrVGG*                                        defaultFontMgr{ nullptr };

  SkFontMgrVGG* registerFont(
    const std::string&       key,
    const fs::path&          fontDir,
    std::vector<std::string> fallbackFonts,
    std::vector<std::string> fallbackEmojiFonts)
  {
    return registerFont(
      key,
      std::vector<fs::path>{ fontDir },
      std::move(fallbackFonts),
      std::move(fallbackEmojiFonts));
  }

  SkFontMgrVGG* registerFont(
    const std::string&       key,
    std::vector<fs::path>    dirs,
    std::vector<std::string> fallbacks,
    std::vector<std::string> fallbackEmojiFonts)
  {
#ifndef NDEBUG
    DEBUG("Font directory: -----------");
    for (const auto& p : dirs)
    {
      DEBUG("%s", p.string().c_str());
    }
#endif
    sk_sp<SkFontMgrVGG> vggFontMgr = VGGFontDirectory(std::move(dirs));
    if (vggFontMgr)
    {
#ifdef VGG_USE_EMBBED_FONT
      if (addEmbbedFont(vggFontMgr.get()))
      {
        fallbacks.push_back(layer::resources::FiraSans::FONT_NAME);
      }
#endif
#ifndef NDEBUG
      DEBUG("----  Fallback font:  -----------");
      for (const auto& f : fallbacks)
      {
        DEBUG("%s", f.c_str());
      }
      for (const auto& f : fallbackEmojiFonts)
      {
        DEBUG("%s", f.c_str());
      }
#endif
      vggFontMgr->setFallbackFonts(std::move(fallbacks));
      vggFontMgr->setFallbackEmojiFonts(std::move(fallbackEmojiFonts));
      auto result = fontMgrs.insert({ key, std::move(vggFontMgr) });
      if (result.second)
      {
        return result.first->second.get();
      }
    }
    return nullptr;
  }
};

FontManager::FontManager()
  : d_ptr(new FontManager__pImpl(this))
{
  auto       font = Config::globalConfig().value("fonts", nlohmann::json{});
  const auto systemFontCfg = Config::genDefaultFontConfig();

  auto arrayEntries = [&](const char* entry)
  {
    std::vector<std::string> result;
    if (font.is_object())
    {
      result = font.value(entry, decltype(result){});
    }
    if (systemFontCfg.is_object())
    {
      result = mergeArrayConfig<std::string>(
        std::move(result),
        systemFontCfg.value(entry, decltype(result){}));
    }
    return result;
  };

  bool atLeastOne = false;

  std::vector<std::string> dirStr(arrayEntries("directory"));
  std::vector<fs::path>    dirs;
  for (const auto& ds : dirStr)
    dirs.push_back(ds);
  std::vector<std::string> fallbackFonts(arrayEntries("fallbackFont"));
  std::vector<std::string> fallbackEmojiFonts(arrayEntries("fallbackEmojiFont"));
  if (!dirs.empty())
  {
    auto mgr = d_ptr->registerFont(
      "default",
      std::move(dirs),
      std::move(fallbackFonts),
      std::move(fallbackEmojiFonts));
    d_ptr->defaultFontMgr = mgr;
    atLeastOne = true;
  }
  if (!atLeastOne)
  {
    if (auto mgr = d_ptr->registerFont("default", fs::path(), {}, {}))
    {
      d_ptr->defaultFontMgr = mgr;
    }
  }
}

SkFontMgrVGG* SkiaFontManagerProxy::skFontMgr() const
{
  return m_mgr.d_ptr->defaultFontMgr;
}

const std::vector<std::string>& FontManager::fallbackFonts() const
{
  ASSERT(d_ptr->defaultFontMgr);
  return d_ptr->defaultFontMgr->fallbackFonts();
}

std::string FontManager::matchFontName(std::string_view inputName) const
{
  auto fontMgr = d_ptr->defaultFontMgr;
  ASSERT(fontMgr);
  auto resolveFromFallback = [](const std::vector<std::string>& candidates)
  {
    for (const auto& candidate : candidates)
    {
      if (const auto components = split(candidate, '-'); !components.empty())
      {
        return components[0];
      }
    }
    return std::string_view();
  };
  std::string fontName;
  if (const auto components = split(inputName, '-'); !components.empty())
  {
    fontName = components[0];
    auto matched = fontMgr->fuzzyMatchFontFamilyName(fontName);
    if (matched)
    {
      INFO(
        "Font [%s] matches real name [%s][%f]",
        fontName.c_str(),
        matched->first.c_str(),
        matched->second);
      fontName = std::string(matched->first.c_str());

      // When font name is provided, we match the real name first.
      // If the score is lower than a threshold, we choose the
      // fallback font rather pick it fuzzily.
      constexpr float THRESHOLD = 70.f;
      if (const auto& fallbackFonts = fontMgr->fallbackFonts();
          !fallbackFonts.empty() && matched->second < THRESHOLD)
      {
        fontName = resolveFromFallback(fallbackFonts);
      }
    }
    else
    {
      DEBUG("No font in font manager");
    }
  }
  else if (const auto& fallbackFonts = fontMgr->fallbackFonts(); !fallbackFonts.empty())
  {
    fontName = resolveFromFallback(fallbackFonts);
  }
  if (fontName.empty())
  {
    // the worst case
    fontName = "FiraSans";
  }
  DEBUG("Given [%s], [%s] is choosed finally", std::string(inputName).c_str(), fontName.c_str());
  return fontName;
}

bool FontManager::addFontFromMemory(const uint8_t* data, size_t size, const char* defaultName)
{
  return d_ptr->defaultFontMgr->addFont(data, size, defaultName, true);
}

FontManager::~FontManager() = default;

} // namespace VGG::layer
