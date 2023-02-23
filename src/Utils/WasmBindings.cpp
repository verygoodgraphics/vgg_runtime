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

#include "Entity/EntityManager.hpp"
#include "Utils/FileManager.hpp"
#include "Utils/Version.hpp"

extern "C"
{
  bool load_file_from_mem(const char* name, unsigned char* data, int len)
  {
    std::vector<unsigned char> buf(data, data + len);
    auto res = VGG::FileManager::loadFileFromMem(buf, name);
    if (auto container = VGG::EntityManager::getEntities())
    {
      container->setRunModeInteractions();
    }
    return res;
  }

  bool is_latest_version(const char* version)
  {
    std::string v1(version);
    std::string v2 = VGG::Version::get();
    return v1 == v2;
  }
};

#endif
