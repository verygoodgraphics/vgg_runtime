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
#ifndef __RENDER_SYSTEM_HPP__
#define __RENDER_SYSTEM_HPP__

#include <core/SkCanvas.h>

#include "Entity/MouseEntity.hpp"
#include "Components/Path.hpp"
#include "Components/Text.hpp"
#include "Components/Relation.hpp"
#include "Components/Styles.hpp"

namespace VGG
{

namespace RenderSystem
{

using PM = CurvePoint::PointMode;

enum class RenderCase
{
  RC_DEFAULT = 0,
  RC_HOVER,
  RC_EDIT,
};

void drawFramedPath(SkCanvas* canvas, const FramedPath& fp, RenderCase rc = RenderCase::RC_DEFAULT);

void drawFramedText(SkCanvas* canvas, const FramedText& ft);

void drawFramedRelation(SkCanvas* canvas, FramedRelation& fr);

void drawEntity(SkCanvas* canvas, Entity& entity);

void drawEntity(SkCanvas* canvas, MouseEntity& me);

}; // namespace RenderSystem

}; // namespace VGG

#endif // __RENDER_SYSTEM_HPP__
