#pragma once

#include <nlohmann/json.hpp>

namespace Config
{

inline nlohmann::json genDefaultFontConfig()
{
  nlohmann::json font = {};
  std::vector<std::string> dirs;
  std::vector<std::string> fallbacks;
#if defined(VGG_HOST_Linux)
  dirs = { "/usr/share/fonts/TTF" };
  fallbacks = { "DejaVuSans" };
#elif defined(VGG_HOST_macOS)
  dirs = { "/System/Library/Fonts/", "~/Library/Fonts/" };
  fallbacks = { "Helvetica" };
#elif defined(VGG_Host_Windows)
  // TODO:: for other platform config
#endif
  font["directory"] = dirs;
  font["fallbackFont"] = fallbacks;
  return font;
}

namespace fs = std::filesystem;
nlohmann::json& globalConfig();

void readGlobalConfig(const fs::path& path);
} // namespace Config
  //
  //
