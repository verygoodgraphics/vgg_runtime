#pragma once
#include <memory>
#include "Application/include/Event/Event.h"
#include "Common/Config.h"
#include "Core/Node.h"
#include "core/SkCanvas.h"
#include "nlohmann/json.hpp"
#include "Scene/EventListener.h"

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
struct VGG_EXPORTS Scene : public EventListener
{
  static ResourceRepo s_resRepo;
  static ObjectTableType s_objectTable;
  static ObjectTableType s_templateObjectTable;
  static InstanceTable s_instanceTable;
  static bool s_enableDrawDebugBound;
  VGG_DECL_IMPL(Scene)
public:
  Scene();
  ~Scene();
  void loadFileContent(const std::string& json);
  void loadFileContent(const nlohmann::json& j);
  void nextArtboard();
  void preArtboard();
  int frameCount() const;
  PaintNode* frame(int index);
  void setPage(int num);
  void nextSymbol();
  void prevSymbol();
  void dispatchEvent(UEvent e) override;
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
