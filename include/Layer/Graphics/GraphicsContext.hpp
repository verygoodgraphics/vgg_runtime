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

#include "Utility/Log.hpp"
#include <memory>
#include <vector>
#include <optional>

namespace VGG::layer
{

class GraphicsLayer;

enum class EGraphicsAPIBackend
{
  API_OPENGL,
  API_VULKAN,
  API_CUSTOM,
};

struct ContextConfig
{
  // int windowSize[2] = { 0, 0 };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

struct ContextProperty
{
  float                              dpiScaling{ 1.0 };
  std::optional<EGraphicsAPIBackend> api;
};

class GraphicsContext : public std::enable_shared_from_this<GraphicsContext>
{
private:
  ContextConfig   m_config;
  ContextProperty m_property;
  friend class GraphicsLayer;

protected:
  // this is the scale factor between window size and the surface size
  float m_resolutionScale{ 1.0 };

protected:
public:
  bool init(const ContextConfig& cfg)
  {
    m_config = cfg;
    auto ok = onInit();
    onInitProperties(m_property);
    return ok;
  }
  const ContextConfig& config() const
  {
    return m_config;
  }

  const ContextProperty& property() const
  {
    return m_property;
  }
  virtual bool  swap() = 0;
  virtual bool  makeCurrent() = 0;
  virtual void  shutdown() = 0;
  virtual void* contextInfo() = 0;
  virtual ~GraphicsContext() = default;

protected:
  virtual bool onInit() = 0;
  virtual void onInitProperties(ContextProperty& property) = 0;
  virtual void onInitContext()
  {
  }
};
} // namespace VGG::layer
