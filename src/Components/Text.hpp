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
#ifndef __TEXT_HPP__
#define __TEXT_HPP__

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Components/Frame.hpp"
#include "Components/Styles.hpp"
#include "Utils/TextCodecs.hpp"
#include "Utils/Types.hpp"

namespace VGG
{

struct FramedText
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(FramedText, frame, text, style);

  Frame frame;
  std::wstring text;
  TextStyle style;

  inline std::vector<std::wstring> getLines() const
  {
    std::vector<std::wstring> ls;
    size_t start = 0;
    for (size_t i = 0; i <= text.size(); i++)
    {
      if (i == text.size() || text[i] == L'\n')
      {
        ls.push_back(text.substr(start, i - start));
        start = i + 1;
      }
    }
    return ls;
  }

  inline std::string getBytes() const
  {
    return TextCodecs::conv.to_bytes(text);
  }
};

}; // namespace VGG

#endif // __TEXT_HPP__
