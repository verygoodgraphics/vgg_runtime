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

namespace VGG
{
namespace Model
{

constexpr auto design_file_name = "design.json";
constexpr auto layout_file_name = "layout.json";
constexpr auto K_SETTINGS_FILE_NAME = "settings.json";
constexpr auto event_listeners_file_name = "event_listeners.json";

constexpr auto ResourcesDir = "resources";
constexpr auto ResourcesDirWithSlash = "resources/";

constexpr auto js_file_suffix = ".mjs";

constexpr auto file_name_key = "fileName";
constexpr auto created_at_key = "createdAt";
constexpr auto K_LAUNCH_FRAME_INDEX = "launchFrameIndex";

} // namespace Model
} // namespace VGG
