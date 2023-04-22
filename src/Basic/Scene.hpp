#pragma once
#include <memory>
#include "include/core/SkCanvas.h"
// #include "vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h"
#include "Basic/Renderer.hpp"
#include "Basic/Loader.hpp"
#include "nlohmann/json.hpp"
#include <fstream>

namespace VGG
{
struct Scene
{
public:
  std::vector<std::vector<std::shared_ptr<PaintNode>>> artboards;
  std::vector<std::shared_ptr<SymbolMasterNode>> symbols;
  int page = 0;
  int symbolIndex = 0;
  bool renderSymbol = false;

  std::map<std::string, std::vector<char>> resources;

  Scene() = default;

  Scene(const std::string& fp)
  {
    LoadFile(fp);
  }

  void LoadFile(const std::string& fp)
  {
    auto size = std::filesystem::file_size(fp.c_str());
    std::vector<char> file_buf(size);
    std::ifstream ifs(fp.c_str(), std::ios_base::binary);
    ifs.read(file_buf.data(), size);
    assert(ifs.gcount() == size);
    nlohmann::json json_out;
    // analyze_sketch_file::analyze(file_buf.data(), size, "hello-sketch", json_out, resources);
    std::ofstream of("vgg_out.json");
    if (of.is_open())
    {
      of << json_out.dump(4);
    }
    LoadFileContent(json_out);
  }

  void LoadFileContent(const std::string& json)
  {
    LoadFileContent(nlohmann::json::parse(json));
  }

  void LoadFileContent(const nlohmann::json& j)
  {
    artboards = fromArtboard(j);
    symbols = fromSymbolMasters(j);
    page = 0;
    symbolIndex = 0;
  }

  void Render(SkCanvas* canvas, int width, int height)
  {
    PaintNode* node = nullptr;
    if (!renderSymbol)
    {
      node = artboards[page][0].get();
    }
    else
    {
      node = symbols[symbolIndex].get();
    }
    SkiaRenderer r;
    r.Draw(canvas, node, width, height);
  }

  void NextArtboard()
  {
    page = (page + 1 >= artboards.size()) ? page : page + 1;
  }

  void PreArtboard()
  {
    page = (page - 1 > 0) ? page - 1 : 0;
  }

  void NextSymbol()
  {
    symbolIndex = (symbolIndex + 1 >= symbols.size()) ? symbolIndex : symbolIndex + 1;
  }

  void PrevSymbol()
  {
    symbolIndex = (symbolIndex - 1 > 0) ? symbolIndex - 1 : 0;
  }
};

}; // namespace VGG
