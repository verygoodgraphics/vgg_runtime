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

constexpr auto K_DESIGN_FILE_NAME = "design.json";
constexpr auto K_LAYOUT_FILE_NAME = "layout.json";
constexpr auto K_SETTINGS_FILE_NAME = "settings.json";
constexpr auto K_EVENT_LISTENERS_FILE_NAME = "event_listeners.json";

constexpr auto K_RESOURCES_DIR = "resources";
constexpr auto K_RESOURCES_DIR_WITH_SLASH = "resources/";

constexpr auto K_JS_FILE_SUFFIX = ".mjs";

constexpr auto K_FILE_NAME_KEY = "fileName";
constexpr auto K_CREATED_AT_KEY = "createdAt";
constexpr auto K_LAUNCH_FRAME_ID = "launchFrameId";

} // namespace Model
} // namespace VGG
