/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef EMSCRIPTEN

#include "BrowserMainComposer.hpp"
#include "Version.h"

extern "C"
{
  bool load_file_from_mem(const char* name, char* data, int len)
  {
    std::vector<char> buf(data, data + len);
    auto& main_composer = VggBrowser::mainComposer();
    return main_composer.controller()->start(buf, "/asset/vgg-format.json");
  }

  bool is_latest_version(const char* version)
  {
    std::string v1(version);
    std::string v2 = VGG::Version::get();
    return v1 == v2;
  }
};

#endif
