#include "Reader/ConfigMananger.h"
#include <filesystem>
#include <fstream>

namespace Config
{

nlohmann::json& globalConfig()
{
  static nlohmann::json config;
  return config;
}

void readGlobalConfig(const fs::path& path)
{
  std::ifstream ifs(path.string());
  if (ifs.is_open())
  {
    auto& cfg = globalConfig();
    ifs >> cfg;
  }
}

} // namespace Config
