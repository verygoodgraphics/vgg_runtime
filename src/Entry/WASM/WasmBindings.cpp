/*
 * Copyright 2021-2023 Chaoya Li <harry75369@gmail.com>
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
#ifdef EMSCRIPTEN

#include "BrowserMainComposer.hpp"
#include "Utility/Version.hpp"

#include <emscripten.h>

extern "C"
{
  bool load_file_from_mem(const char* name, char* data, int len)
  {
    std::vector<char> buf(data, data + len);
    auto              controller = VggBrowser::mainComposer().controller();
    return controller->start(buf, "/asset/vgg-format.json", "/asset/vgg_layout.json");
  }

  bool is_latest_version(const char* version)
  {
    std::string v1(version);
    std::string v2 = VGG::Version::get();
    return v1 == v2;
  }

  // Use current daruma file as an editor.
  // data/len: is the daruma file to be edited.
  // top/right/bottom/left: offsets within the editor
  EMSCRIPTEN_KEEPALIVE bool edit(char* data, int len, int top, int right, int bottom, int left)
  {
    auto& mainComposer = VggBrowser::mainComposer();
    mainComposer.enableEdit(top, right, bottom, left);

    std::vector<char> buffer(data, data + len);
    return mainComposer.controller()->edit(buffer);
  }

  EMSCRIPTEN_KEEPALIVE void listenAllEvents(bool enabled)
  {
    auto& mainComposer = VggBrowser::mainComposer();

    return mainComposer.controller()->listenAllEvents(enabled);
  }
};

#endif
