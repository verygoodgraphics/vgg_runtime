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
#include "Layer/Core/RenderNode.hpp"
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Config.hpp"
#include "Layer/Effects.hpp"
#include "Layer/Graphics/GraphicsLayer.hpp"
#include "Layer/Scene.hpp"

#include <vector>
class SkCanvas;
class SkPicture;
class SkSurface;

template<typename T>
class sk_sp;

namespace VGG::layer
{

enum class EImageEncode
{
  IE_PNG,
  IE_JPEG,
  IE_WEBP,
  IE_RAW
};

struct ImageOptions
{
  EImageEncode encode;
  int          position[2] = { 0, 0 };
  int          extend[2] = { 0, 0 };
  int          quality{ 100 };
};

struct SVGOptions
{
  int position[2];
  int extend[2];
};

struct PDFOptions
{
  int position[2];
  int extend[2];
};

#define DEPRECATED(msg) [[deprecated(msg)]]

class VLayer__pImpl;
class VGG_EXPORTS VLayer : public GraphicsLayer
{
  VGG_DECL_IMPL(VLayer);
  int m_position[2] = { 0, 0 };

protected:
  virtual std::optional<ELayerError> onInit() override;

public:
  VLayer();
  ~VLayer();
  virtual void beginFrame() override;
  virtual void render() override;
  virtual void endFrame() override;
  virtual void shutdown() override;

  void resize(int w, int h) override;
  DEPRECATED("use setRenderNode") void addRenderItem(std::shared_ptr<Renderable> item);

  void       setBackgroundColor(uint32_t color);
  void       setScaleFactor(float scale);
  float      scaleFactor() const;
  PaintNode* nodeAt(int x, int y);
  void       nodeAt(int x, int y, PaintNode::NodeVisitor visitor);

  void setRenderNode(Ref<ZoomerNode> transform, Ref<RenderNode> node);

  void setRenderNode(Ref<RenderNode> node); // This is recommanded

  SkCanvas* layerCanvas();
  void      clearLayerCanvas();

  void setDrawClickBounds(bool enable);
  bool enableDrawClickBounds() const;
  void setDrawPositionEnabled(bool enable);
  bool enableDrawPosition() const;

  void drawPosition(int x, int y)
  {
    m_position[0] = x;
    m_position[1] = y;
  }

  void setSamplingOptions(const SkSamplingOptions& opt)
  {
    setGlobalSamplingOptions(opt);
  }

  std::optional<std::vector<char>> makeImageSnapshot(const ImageOptions& opts);
  void                             makeImageSnapshot(const ImageOptions& opts, std::ostream& os);
  std::optional<std::vector<char>> makeSKP();
  void                             makeSKP(std::ostream& os);
  sk_sp<SkPicture>                 makeSkPicture(int width, int height);
};

} // namespace VGG::layer
