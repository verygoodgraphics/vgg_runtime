#pragma once

#include <nlohmann/json.hpp>

namespace Config
{

namespace fs = std::filesystem;
nlohmann::json& globalConfig();

void readGlobalConfig(const fs::path& path);
} // namespace Config
