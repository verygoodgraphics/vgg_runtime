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
#ifndef __VERSION_HPP__
#define __VERSION_HPP__

#include "Log.h"

namespace VGG
{

struct Version
{
  inline static std::string get()
  {
#ifdef GIT_SHA1
    return strlimit(XSTR(GIT_SHA1), 8, "");
#else
    return "develop";
#endif
  }
};

}; // namespace VGG

#endif // __VERSION_HPP__
