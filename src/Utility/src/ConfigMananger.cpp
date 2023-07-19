#include "ConfigMananger.h"
#include <filesystem>
#include <fstream>

namespace Config
{
static nlohmann::json config;

nlohmann::json& globalConfig()
{
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
