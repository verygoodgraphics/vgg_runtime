/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include <nlohmann/json.hpp>
#include <cstdlib>

namespace Config
{

inline nlohmann::json genDefaultFontConfig()
{
  nlohmann::json           font = {};
  std::vector<std::string> dirs;
  std::vector<std::string> fallbacks;
  std::vector<std::string> emojiFallback;
#if defined(VGG_TARGET_PLATFORM_Linux)
  dirs = { "/usr/share/fonts" };
  fallbacks = { "DejaVuSans" };
  emojiFallback = {"Apple Color Emoji"};
#elif defined(VGG_TARGET_PLATFORM_macOS)
  dirs = { "/System/Library/Fonts/",
           std::filesystem::path(std::getenv("HOME")) / "Library" / "Fonts" };
  fallbacks = { "Helvetica" };
  emojiFallback = {"Apple Color Emoji"};
#elif defined(VGG_TARGET_PLATFORM_Windows)
  // TODO:: for other platform config
#endif
  font["directory"] = dirs;
  font["fallbackFont"] = fallbacks;
  font["defaultEmojiFont"] = emojiFallback;
  return font;
}

namespace fs = std::filesystem;
nlohmann::json& globalConfig();

void readGlobalConfig(const fs::path& path);
} // namespace Config
  //
  //
