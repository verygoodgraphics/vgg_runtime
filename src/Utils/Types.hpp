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
#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <nlohmann/json.hpp>
#include <optional>
#include <variant>

#include "Utils/TextCodecs.hpp"

namespace VGG
{

class Uncopyable
{
public:
  Uncopyable() = default;

  Uncopyable(const Uncopyable&) = delete;

  Uncopyable& operator=(const Uncopyable&) = delete;

  Uncopyable(Uncopyable&&) = default;

  Uncopyable& operator=(Uncopyable&&) = default;
};

class Unmovable
{
public:
  Unmovable() = default;

  Unmovable(const Unmovable&) = default;

  Unmovable& operator=(const Unmovable&) = default;

  Unmovable(Unmovable&&) = delete;

  Unmovable& operator=(Unmovable&&) = delete;
};

}; // namespace VGG

namespace nlohmann
{

template<typename T>
struct adl_serializer<std::optional<T>>
{
  static void to_json(json& j, const std::optional<T>& opt)
  {
    if (opt.has_value())
    {
      j = *opt;
    }
    else
    {
      j = nullptr;
    }
  }

  static void from_json(const json& j, std::optional<T>& opt)
  {
    if (j.is_null())
    {
      opt = std::nullopt;
    }
    else
    {
      opt = j.get<T>();
    }
  }
};

template<>
struct adl_serializer<std::wstring>
{
  static void to_json(json& j, const std::wstring& s)
  {
    j = VGG::TextCodecs::conv.to_bytes(s);
  }

  static void from_json(const json& j, std::wstring& s)
  {
    s = VGG::TextCodecs::conv.from_bytes(j.get<std::string>());
  }
};

}; // namespace nlohmann

#endif // __TYPES_HPP__
