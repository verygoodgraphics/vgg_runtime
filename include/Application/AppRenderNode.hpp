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
#include <cctype>
#include <core/SkColor.h>

#include "Layer/Core/RenderNode.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Renderer.hpp"

namespace VGG::layer
{
class Renderer;
}

namespace VGG::app
{

class AppRenderNode : public VGG::layer::RenderNode
{
public:
  AppRenderNode(layer::VRefCnt* cnt, layer::Ref<layer::SceneNode> sceneNode)
    : RenderNode(cnt, EState::INVALIDATE)
    , m_sceneNode(std::move(sceneNode))
  {
    observe(m_sceneNode);
  }

  layer::SceneNode* getSceneNode()
  {
    return m_sceneNode.get();
  }

  void render(layer::Renderer* render) override
  {
    auto canvas = render->canvas();
    canvas->clear(SK_ColorBLUE);
    // TODO:: WRITE YOUR OWN CODE HERE

    m_sceneNode->render(render);
  }

  Bounds effectBounds() const override
  {
    return m_sceneNode->effectBounds();
  }

  Bounds onRevalidate() override
  {
    auto b = m_sceneNode->revalidate();
    return b;
  }

  SkPicture* picture() const override
  {
    return m_sceneNode->picture();
  }

  ~AppRenderNode() override
  {
    unobserve(m_sceneNode);
  }

  VGG_CLASS_MAKE(AppRenderNode)

private:
  layer::Ref<layer::SceneNode> m_sceneNode;
};

} // namespace VGG::app
