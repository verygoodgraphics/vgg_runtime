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
#ifndef __VERSION_HPP__
#define __VERSION_HPP__

#include "Utility/Log.hpp"

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
