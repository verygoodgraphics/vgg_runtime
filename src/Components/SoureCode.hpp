/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
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
#ifndef __SOURCE_CODE_HPP__
#define __SOURCE_CODE_HPP__

#include <nlohmann/json.hpp>
#include <string>

namespace VGG
{

struct SourceCode
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SourceCode, type, content);

  enum Language
  {
    JS = 0,
    SKSL,
    NUM_LANGS,
  };

  Language type{ JS };
  std::string content;
};

}; // namespace VGG

#endif // __SOURCE_CODE_HPP__
