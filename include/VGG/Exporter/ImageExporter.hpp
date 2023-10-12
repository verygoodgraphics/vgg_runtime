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
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <tuple>
#include <map>
#include <nlohmann/json.hpp>

namespace VGG::exporter
{

struct ImageOption
{
  int imageQuality = 100;
  int resolutionLevel = 2;
};

enum class EBackend
{
  VULKAN,
};
struct BackendInfo
{
  EBackend type;
  int majorVersion;
  int minorVersion;
};

struct ExporterInfo
{
  std::string buildCommit;
  BackendInfo backend;
};

using OutputCallback = std::function<bool(const std::string&, const std::vector<char>&)>;
using Resource = std::map<std::string, std::vector<char>>;

class Exporter__pImpl;
class Exporter
{
  std::unique_ptr<Exporter__pImpl> d_impl; // NOLINT
public:
  class Iterator__pImpl;
  class Iterator
  {
    std::unique_ptr<Iterator__pImpl> d_impl; // NOLINT
    Iterator(Exporter& exporter, nlohmann::json json, Resource resource, const ImageOption& opt);
    friend class Exporter;

  public:
    bool next(std::string& key, std::vector<char>& image);
    Iterator(Iterator&& other) noexcept;
    Iterator& operator=(Iterator&& other) noexcept;
    ~Iterator();
  };

  Exporter();
  void info(ExporterInfo* info);

  Iterator render(nlohmann::json j,
                  std::map<std::string, std::vector<char>> resources,
                  const ImageOption& opt)
  {
    return Exporter::Iterator{ *this, std::move(j), std::move(resources), opt };
  }
  void setOutputCallback(OutputCallback callback);
  ~Exporter();
};

void setGlobalConfig(const std::string& fileName);

}; // namespace VGG::exporter
