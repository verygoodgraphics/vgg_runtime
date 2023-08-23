#pragma once
#include <memory>
#include "Scene/GraphicsLayer.h"
#include "Common/Config.h"
#include "Core/Node.h"
#include "core/SkCanvas.h"
#include "nlohmann/json.hpp"
#include "Scene/Renderable.h"
#include "Zoomer.h"

class SkCanvas;

namespace VGG
{
class SymbolMasterNode;
class PaintNode;
using ResourceRepo = std::map<std::string, std::vector<char>>;
using ObjectTableType = std::unordered_map<std::string, std::weak_ptr<PaintNode>>;
using ObjectTableType = std::unordered_map<std::string, std::weak_ptr<PaintNode>>;
using InstanceTable =
  std::unordered_map<std::string,
                     std::pair<std::weak_ptr<PaintNode>,
                               std::string>>; // {instance_id: {instance_object: master_id}}
                                              //
struct NodeContainer
{
public:
  std::vector<std::shared_ptr<PaintNode>> frames;
  std::vector<std::shared_ptr<PaintNode>> symbols;
  NodeContainer() = default;
  NodeContainer(std::vector<std::shared_ptr<PaintNode>> frames,
                std::vector<std::shared_ptr<PaintNode>> symbols)
    : frames(std::move(frames))
    , symbols(std::move(symbols))
  {
  }
};
class Scene__pImpl;
struct VGG_EXPORTS Scene : public layer::Renderable
{
  static ResourceRepo s_resRepo;
  static ObjectTableType s_objectTable;
  static ObjectTableType s_templateObjectTable;
  static InstanceTable s_instanceTable;
  static bool s_enableDrawDebugBound;

private:
  VGG_DECL_IMPL(Scene)
  std::string m_name{ "Default Scene" };

protected:
  void onRender(SkCanvas* canvas) override;

public:
  Scene();
  ~Scene();
  void loadFileContent(const std::string& json);
  void loadFileContent(const nlohmann::json& j);
  void setName(std::string name)
  {
    m_name = std::move(name);
  }
  const std::string name() const
  {
    return m_name;
  }
  void nextArtboard();
  void preArtboard();
  int frameCount() const;
  PaintNode* frame(int index);
  void setPage(int num);
  void nextSymbol();
  void prevSymbol();
  // To remove zoomer, just set nullptr
  void setZoomer(std::shared_ptr<Zoomer> zoomer);
  Zoomer* zoomer();
  static ResourceRepo& getResRepo()
  {
    return s_resRepo;
  }

  static ObjectTableType& getObjectTable()
  {
    return s_objectTable;
  }

  static ObjectTableType& templateObjectTable()
  {
    return s_templateObjectTable;
  }

  static InstanceTable& instanceObjects()
  {
    return s_instanceTable;
  }

  static void setResRepo(std::map<std::string, std::vector<char>> repo);

  static void enableDrawDebugBound(bool enable)
  {
    s_enableDrawDebugBound = enable;
  }

  static bool isEnableDrawDebugBound()
  {
    return s_enableDrawDebugBound;
  }

private:
  void preprocessMask(PaintNode* node);
  void instantiateTemplates();
};

}; // namespace VGG
