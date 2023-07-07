#pragma once

#include <nlohmann/json.hpp>

namespace Config
{

namespace fs = std::filesystem;
nlohmann::json& GetGlobalConfig();

void ReadGlobalConfig(const fs::path& path);
} // namespace Config
