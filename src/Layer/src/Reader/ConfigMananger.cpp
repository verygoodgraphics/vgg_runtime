#include "Reader/ConfigMananger.h"
#include <filesystem>
#include <fstream>

namespace Config
{

nlohmann::json& GetGlobalConfig()
{
  static nlohmann::json config;
  return config;
}

void ReadGlobalConfig(const fs::path& path)
{
  std::ifstream ifs(path.string());
  if (ifs.is_open())
  {
    auto& cfg = GetGlobalConfig();
    ifs >> cfg;
  }
}

} // namespace Config
