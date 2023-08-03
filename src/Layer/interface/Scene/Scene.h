#pragma once
#include <memory>
#include "Common/Config.h"
#include "Core/Node.h"
#include "core/SkCanvas.h"
#include "nlohmann/json.hpp"

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

struct VGG_EXPORTS Scene
{
  static ResourceRepo s_resRepo;
  static ObjectTableType s_objectTable;
  static ObjectTableType s_templateObjectTable;
  static InstanceTable s_instanceTable;
  static bool s_enableDrawDebugBound;

  std::vector<std::shared_ptr<PaintNode>> artboards;
  std::vector<std::shared_ptr<PaintNode>> symbols;

public:
  int page{ 0 };
  int symbolIndex{ 0 };
  bool renderSymbol{ false };
  bool maskDirty{ true };
  Scene();
  void loadFileContent(const std::string& json);
  void loadFileContent(const nlohmann::json& j);
  void render(SkCanvas* canvas);
  void nextArtboard();
  void preArtboard();
  void setPage(int num);
  void nextSymbol();
  void prevSymbol();

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

  static void setResRepo(std::map<std::string, std::vector<char>> repo)
  {
    Scene::s_resRepo = std::move(repo);
  }

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
