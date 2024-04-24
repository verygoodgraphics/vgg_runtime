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
#include "Layer/Renderable.hpp"
#include "Layer/Core/RasterCache.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Config.hpp"

#include <map>
#include <memory>
class SkCanvas;
class GrRecordingContext;

namespace VGG
{
namespace layer
{
class PaintNode;
using PaintNodePtr = VGG::layer::Ref<PaintNode>;
using PaintNodeRef = VGG::layer::WeakRef<PaintNode>;

class VLayer;
class Rasterizer;
} // namespace layer

class Scene__pImpl;
class VGG_EXPORTS [[deprecated("Using SceneNode instead")]] Scene : public layer::Renderable
{
  friend class layer::VLayer; // Temporary support for VLayer

public:
  Scene(std::unique_ptr<layer::Rasterizer> cache = nullptr);
  virtual ~Scene();
  void setSceneRoots(std::vector<layer::FramePtr> roots);
  void setName(std::string name)
  {
    m_name = std::move(name);
  }
  const std::string& name() const
  {
    return m_name;
  }
  void          nextArtboard();
  void          preArtboard();
  int           frameCount() const;
  layer::FrameNode* frame(int index);
  void          setPage(int num);
  int           currentPage() const;
  // To remove zoomer, just set nullptr
  void          setZoomer(std::shared_ptr<Zoomer> zoomer);
  Zoomer*       zoomer();

  void nodeAt(int x, int y, layer::PaintNode::NodeVisitor visitor);

  void enableDrawDebugBounds(bool enabled);
  bool isEnableDrawDebugBounds();

  void onViewportChange(const Bounds& bounds);

  [[deprecated("Use setGlobalResourceProvider")]] static void setResRepo(
    std::map<std::string, std::vector<char>> repo);

protected:
  void onRender(SkCanvas* canvas) override;
  void onZoomScaleChanged(Zoomer::Scale value);
  void onZoomTranslationChanged(float x, float y);
  void setOwner(layer::VLayer* owner);

  void onRevalidate();

private:
  void invalidateMask();
  void invalidate();

  friend class ::Zoomer;
  VGG_DECL_IMPL_REF(Scene)
  std::string m_name{ "Default Scene" };
};

}; // namespace VGG
