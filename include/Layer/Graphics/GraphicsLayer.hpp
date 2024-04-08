/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "GraphicsContext.hpp"
#include <memory>
#include <optional>

namespace VGG::layer
{

enum class ELayerError
{
  TEXTURE_SIZE_OUT_OF_RANGE,
  MAKE_CURRENT_CONTEXT_ERROR,
  RENDER_ENGINE_ERROR,
  UNKNOWN_ERROR,
};

class GraphicsLayer : public std::enable_shared_from_this<GraphicsLayer>
{
  layer::GraphicsContext* m_ctx{ nullptr };

public:
  std::optional<ELayerError> init(layer::GraphicsContext* ctx)
  {
    m_ctx = ctx;
    return onInit();
  }
  layer::GraphicsContext* context()
  {
    return m_ctx;
  }
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;

  // After init, you must call resize with proper value so that render() could be called
  virtual void resize(int w, int h) = 0;
  virtual ~GraphicsLayer() = default;

protected:
  virtual std::optional<ELayerError> onInit() = 0;
};

} // namespace VGG::layer
