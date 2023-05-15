#pragma once
#include <memory>
#include "Basic/Node.hpp"
#include "include/core/SkCanvas.h"
#include "nlohmann/json.hpp"

namespace VGG
{
class SymbolMasterNode;
class ArtboardNode;
class PaintNode;
using ResourceRepo = std::map<std::string, std::vector<char>>;
using ObjectTableType = std::unordered_map<std::string, std::weak_ptr<PaintNode>>;
struct Scene
{
  static ResourceRepo ResRepo;
  static ObjectTableType ObjectTable;

public:
  std::vector<std::shared_ptr<ArtboardNode>> artboards;
  std::vector<std::shared_ptr<SymbolMasterNode>> symbols;
  int page{ 0 };
  int symbolIndex{ 0 };
  bool renderSymbol{ false };
  bool maskDirty{ true };
  Scene() = default;
  void LoadFileContent(const std::string& json);
  void LoadFileContent(const nlohmann::json& j);
  void Render(SkCanvas* canvas);
  void NextArtboard();
  void PreArtboard();
  void SetPage(int num);
  void NextSymbol();
  void PrevSymbol();

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

private:
  void preprocessMask(PaintNode* node);
};

}; // namespace VGG
