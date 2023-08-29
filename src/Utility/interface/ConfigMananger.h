#pragma once

#include <nlohmann/json.hpp>

namespace Config
{

inline nlohmann::json genDefaultConfig()
{
  nlohmann::json cfg{};
  nlohmann::json font = {};
  std::string path;
  std::vector<std::string> fallbacks;
#if defined(VGG_HOST_Linux)
  path = "/usr/share/fonts/TTF";
  fallbacks = { "DejaVuSans" };
#elif defined(VGG_HOST_macOS)
  path = "/System/Library/Fonts/";
  fallbacks = { "Helvetica" };
#endif
  font["dir"] = path;
  font["fallbackFonts"] = fallbacks;
  cfg["useEmbbedFont"] = true;
  cfg["fonts"]["fontCollections"]["vgg_font"] = font;
  cfg["fonts"]["defaultFontCollection"] = "vgg_font";
  return cfg;
}

namespace fs = std::filesystem;
nlohmann::json& globalConfig();

void readGlobalConfig(const fs::path& path);
} // namespace Config
  //
  //
