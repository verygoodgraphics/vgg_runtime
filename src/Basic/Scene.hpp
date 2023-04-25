#pragma once
#include <memory>
#include "include/core/SkCanvas.h"
// #include "vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h"
#include "nlohmann/json.hpp"
#include "Basic/RenderTreeDef.hpp"

namespace VGG
{
class SymbolMasterNode;

class ArtboardNode;

using ResourceRepo = std::map<std::string, std::vector<char>>;
struct Scene
{
public:
  static ResourceRepo ResRepo;
  std::vector<std::shared_ptr<ArtboardNode>> artboards;
  std::vector<std::shared_ptr<SymbolMasterNode>> symbols;
  int page = 0;
  int symbolIndex = 0;
  bool renderSymbol = false;
  Scene() = default;
  void LoadFileContent(const std::string& json);
  void LoadFileContent(const nlohmann::json& j);
  void Render(SkCanvas* canvas);
  void NextArtboard();
  void PreArtboard();
  void NextSymbol();
  void PrevSymbol();
};

}; // namespace VGG
