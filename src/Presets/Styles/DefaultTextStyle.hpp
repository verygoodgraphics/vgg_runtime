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
#ifndef __DEFAULT_TEXT_STYLE_HPP__
#define __DEFAULT_TEXT_STYLE_HPP__

#include "Components/Styles.hpp"

namespace VGG
{

class DefaultTextStyle
{
public:
  static inline TextStyle create()
  {
    return TextStyle{};
  }

  static inline TextStyle createCentered()
  {
    return TextStyle{
      .horzAlignment = TextStyle::HorzAlignment::HA_CENTER,
      .vertAlignment = TextStyle::VertAlignment::VA_CENTER,
    };
  }
};

}; // namespace VGG

#endif // __DEFAULT_TEXT_STYLE_HPP__
