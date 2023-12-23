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
#include "Layer/Renderable.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Config.hpp"

#include <nlohmann/json.hpp>

#include <map>
#include <memory>
class SkCanvas;

namespace VGG
{
namespace layer
{
class PaintNode;
#ifdef USE_SHARED_PTR
using PaintNodePtr = std::shared_ptr<PaintNode>;
using PaintNodeRef = std::weak_ptr<PaintNode>;
#else
using PaintNodePtr = VGG::layer::Ref<PaintNode>;
using PaintNodeRef = VGG::layer::WeakRef<PaintNode>;
#endif
} // namespace layer
using namespace layer;

using ResourceRepo = std::map<std::string, std::vector<char>>;
using ObjectTableType = std::unordered_map<std::string, PaintNodeRef>;
using InstanceTable = std::unordered_map<
  std::string,
  std::pair<
    PaintNodeRef,
    std::string>>; // {instance_id: {instance_object: master_id}}
                   //
class Scene__pImpl;
class VGG_EXPORTS Scene : public layer::Renderable
{
  static ResourceRepo s_resRepo;

private:
  VGG_DECL_IMPL(Scene)
  std::string m_name{ "Default Scene" };

protected:
  void onRender(SkCanvas* canvas) override;

public:
  Scene();
  virtual ~Scene();
  [[deprecated("Use SceneBuilder for data preparation")]] void loadFileContent(
    const nlohmann::json& j);
  void setSceneRoots(std::vector<PaintNodePtr> roots);
  void setName(std::string name)
  {
    m_name = std::move(name);
  }
  const std::string name() const
  {
    return m_name;
  }
  void       nextArtboard();
  void       preArtboard();
  int        frameCount() const;
  PaintNode* frame(int index);
  void       setPage(int num);
  void       nextSymbol();
  void       prevSymbol();
  int        currentPage() const;
  // To remove zoomer, just set nullptr
  void       setZoomer(std::shared_ptr<Zoomer> zoomer);
  Zoomer*    zoomer();
  void       enableDrawDebugBound(bool enabled);
  bool       isEnableDrawDebugBound();

  void                 repaint();
  static ResourceRepo& getResRepo()
  {
    return s_resRepo;
  }

  static void setResRepo(std::map<std::string, std::vector<char>> repo);

private:
  void preprocessMask(PaintNode* node);
};

}; // namespace VGG
