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
#ifndef __ENTITY_DEFS_HPP__
#define __ENTITY_DEFS_HPP__

#include <memory>

namespace VGG
{

struct Entity;

using EntityRaw = Entity*;

using EntityPtr = std::shared_ptr<Entity>;

using EntityRef = std::weak_ptr<Entity>;

}; // namespace VGG

#endif // __ENTITY_DEFS_HPP__
