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
#ifndef __DEFAULT_PATH_STYLE_HPP__
#define __DEFAULT_PATH_STYLE_HPP__

#include "Components/Styles.hpp"

namespace VGG
{

class DefaultPathStyle
{
public:
  static inline PathStyle create()
  {
    return PathStyle{
      .fills = {
        StyleSwitch<FillStyle>{
          .isEnabled = true,
        },
      },
      .borders = {
        StyleSwitch<BorderStyle>{
          .isEnabled = true,
        },
      },
    };
  }

  static inline PathStyle createCursor()
  {
    auto ps = DefaultPathStyle::create();
    ps.fills[0].style.color = Colors::White;
    ps.borders[0].style.color = Colors::Black;
    return ps;
  }

  static inline PathStyle createForImage(const std::string& name)
  {
    return PathStyle{
      .fills = {
        StyleSwitch<FillStyle>{
          .isEnabled = true,
          .style = FillStyle{
            .fillType = FillStyle::FillType::IMAGE,
            .imageName = name,
            .imageFillType = FillStyle::ImageFillType::STRETCH,
          },
        },
      },
    };
  }

  static inline PathStyle createForLine()
  {
    auto ps = DefaultPathStyle::create();
    ps.fills[0].isEnabled = false;
    return ps;
  }

  static inline PathStyle createForPreview()
  {
    return PathStyle{
      .borders = {
        StyleSwitch<BorderStyle>{
          .isEnabled = true,
        },
      },
    };
  }
};

}; // namespace VGG

#endif // __DEFAULT_PATH_STYLE_HPP__
