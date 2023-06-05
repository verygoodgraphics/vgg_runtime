#pragma once
#include "IReader.hpp"
#include "Utils/Utils.hpp"
#include <exception>
#include <filesystem>
#include <vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h>

#include <fstream>
namespace VGG
{

class SketchFileReader : public IReader
{
  std::filesystem::path filename;
  std::map<std::string, std::vector<char>> resources;

public:
  SketchFileReader(const std::string& filename)
    : filename(filename)
  {
  }

  nlohmann::json readFormat() override
  {
    auto size = std::filesystem::file_size(prefix / filename);
    std::vector<char> file_buf(size);
    std::ifstream ifs(prefix / filename, std::ios_base::binary);
    if (ifs.is_open() == true)
    {
      ifs.read(file_buf.data(), size);
      assert(ifs.gcount() == size);

      nlohmann::json json_out;
      try
      {
        analyze_sketch_file::analyze(file_buf.data(), size, "vgg_format", json_out, resources);
      }
      catch (const std::exception& e)
      {
        WARN("%s", e.what());
        return {};
      }
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
