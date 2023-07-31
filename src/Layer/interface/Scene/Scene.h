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

struct SceneConfig
{
};

struct VGG_EXPORTS Scene
{
  static ResourceRepo ResRepo;
  static ObjectTableType ObjectTable;
  static bool s_enableDrawDebugBound;

  std::vector<std::shared_ptr<PaintNode>> artboards;
  std::vector<std::shared_ptr<SymbolMasterNode>> symbols;

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
    return ResRepo;
  }

  static ObjectTableType& getObjectTable()
  {
    return ObjectTable;
  }

  static void setResRepo(std::map<std::string, std::vector<char>> repo)
  {
    Scene::ResRepo = std::move(repo);
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
};

}; // namespace VGG
