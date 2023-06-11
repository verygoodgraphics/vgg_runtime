#include "Scene/Scene.h"
#include "Reader/IReader.hpp"
#include "Reader/ReaderFactory.h"
#include <functional>
#include <filesystem>
#include <map>
#include <vector>

namespace fs = std::filesystem;
using namespace VGG;

inline bool load(const fs::path& filepath,
                 const fs::path& datapath,
                 const fs::path& prefix,
                 std::function<void(const nlohmann::json& json,
                                    std::map<std::string, std::vector<char>> res)> loadCallback)
{
  auto fp = filepath;
  auto ext = fp.extension().string();
  std::shared_ptr<IReader> reader;
  if (ext == ".sketch")
  {
    reader = GetSketchReader(fp.string());
  }
  else if (ext == ".json")
  {
    std::string resFile = fp.stem().string(); // same with filename as default
    reader = GetRawReader(fp.string(), datapath.string());
  }

  if (reader)
  {
    reader->setPrefix(prefix);
    nlohmann::json json = reader->readFormat();
    auto res = reader->readResource();
    loadCallback(json, std::move(res));
  }
  else
  {
    INFO("Failed to initialize a reader\n");
  }
  return true;
}
