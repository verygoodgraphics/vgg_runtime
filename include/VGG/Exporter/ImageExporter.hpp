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
#include "Type.hpp"

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <tuple>
#include <map>
#include <nlohmann/json.hpp>

namespace VGG::exporter
{

struct ExporterInfo
{
  std::string buildCommit;
  std::string graphicsInfo;
};

class ImageIteratorImpl;
class Exporter;
class ImageIterator
{
  std::unique_ptr<ImageIteratorImpl> d_impl; // NOLINT
  ImageIterator(Exporter& exporter,
                nlohmann::json design,
                nlohmann::json layout,
                Resource resource,
                const ImageOption& opt);
  friend class Exporter;
  const ImageOption m_opts;

public:
  bool next(std::string& key, std::vector<char>& image);
  ImageIterator(ImageIterator&& other) noexcept;
  ImageIterator& operator=(ImageIterator&& other) noexcept = delete;
  ~ImageIterator();
};

class Exporter__pImpl;
class Exporter
{
  std::unique_ptr<Exporter__pImpl> d_impl; // NOLINT
  friend class ImageIteratorImpl;

public:
  Exporter();
  void info(ExporterInfo* info);

  ImageIterator render(nlohmann::json design,
                       nlohmann::json layout,
                       std::map<std::string, std::vector<char>> resources,
                       const ImageOption& opt)
  {
    return ImageIterator{ *this, std::move(design), std::move(layout), std::move(resources), opt };
  }

  void setOutputCallback(OutputCallback callback);
  ~Exporter();
};

void setGlobalConfig(const std::string& fileName);

}; // namespace VGG::exporter
