/*
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
#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <optional>
using OutputCallback = std::function<bool(const std::string&, const std::vector<char>&)>;
using Resource = std::map<std::string, std::vector<char>>;

namespace VGG::exporter
{
enum EFileType
{
  SVG,
  PDF,
  SKP
};

enum EImageType
{
  PNG,
  JPEG,
  WEBP,
};

struct ImageOption
{
  int        imageQuality = 100;
  int        resolutionLevel = 2;
  EImageType type{ EImageType::PNG };
};

enum class EBackend
{
  VULKAN,
};

class IteratorResult
{
  bool m_hasNext{ false };

public:
  IteratorResult(bool hasNext)
    : m_hasNext(hasNext)
  {
  }
  enum EResult
  {
    // No value yet
  };
  struct TimeCost
  {
    // Connting in seconds
    float render{ 0.f };
    float encode{ 0.f }; // This time consuming is the sum of capturing and encoding stages
    TimeCost(float render = 0.f, float encode = 0.f)
      : render(render)
      , encode(encode)
    {
    }
    TimeCost(const TimeCost& other) = default;
    TimeCost(TimeCost&& other) noexcept = default;
    TimeCost& operator=(const TimeCost& other) = default;
    TimeCost& operator=(TimeCost&& other) noexcept = default;
    TimeCost& operator+=(const TimeCost& other)
    {
      render += other.render;
      encode += other.encode;
      return *this;
    }
    TimeCost operator+(const TimeCost& other) const
    {
      TimeCost s(*this);
      s += other;
      return s;
    }
  };
  using ExportData = std::pair<std::string, std::vector<char>>;

  std::optional<EResult>    type;
  std::optional<TimeCost>   timeCost;
  std::optional<ExportData> data;

  bool hasNext() const
  {
    return m_hasNext;
  };
  operator bool() const
  {
    return hasNext();
  }
};

struct BuilderResult
{
  enum EResult
  {
    VERSION_MISMATCH,
  };

  struct TimeCost
  {
    // Counting in seconds
    float expand{ 0.f };
    float layout{ 0.f };
    TimeCost(float render = 0.f, float encode = 0.f)
      : expand(render)
      , layout(encode)
    {
    }
    TimeCost(const TimeCost& other) = default;
    TimeCost(TimeCost&& other) noexcept = default;
    TimeCost& operator=(const TimeCost& other) = default;
    TimeCost& operator=(TimeCost&& other) noexcept = default;
    TimeCost& operator+=(const TimeCost& other)
    {
      expand += other.expand;
      layout += other.layout;
      return *this;
    }
    TimeCost operator+(const TimeCost& other) const
    {
      TimeCost s(*this);
      s += other;
      return s;
    }
  };
  std::optional<EResult>  type;
  std::optional<TimeCost> timeCost;
};

struct ExportOption
{
  bool enableExpand{ true };
  bool enableLayout{ true };
};

} // namespace VGG::exporter
