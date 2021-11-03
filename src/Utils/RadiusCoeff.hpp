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
#ifndef __RADIUS_COEFF_HPP__
#define __RADIUS_COEFF_HPP__

namespace VGG
{

struct RadiusCoeff
{
  static constexpr double maxCoeff = 10.;
  static constexpr double minCoeff = -10.;
  inline static double coeff = 0.81;
};

}; // namespace VGG

#endif // __RADIUS_COEFF_HPP__
