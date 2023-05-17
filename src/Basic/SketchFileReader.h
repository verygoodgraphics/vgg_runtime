#pragma once
#include "IReader.hpp"
#include <vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h>

#include <fstream>
namespace VGG
{

class SketchFileReader : public IReader
{
  std::string filename;
  std::map<std::string, std::vector<char>> resources;

public:
  SketchFileReader(const std::string& filename)
    : filename(filename)
  {
  }

  nlohmann::json readFormat() override
  {
    auto size = std::filesystem::file_size(filename);
    std::vector<char> file_buf(size);
    std::ifstream ifs(filename, std::ios_base::binary);
    if (ifs.is_open() == true)
    {
      ifs.read(file_buf.data(), size);
      assert(ifs.gcount() == size);

      nlohmann::json json_out;
      analyze_sketch_file::analyze(file_buf.data(), size, "vgg_format", json_out, resources);
      auto str = json_out.dump();
      std::ofstream ofs("out.json");
      ofs.write(str.c_str(), str.size());
      ofs.close();
      return json_out;
    }
    return {};
  }

  std::map<std::string, std::vector<char>> readResource() override
  {
    return resources;
  }
};
} // namespace VGG
