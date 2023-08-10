#include "daruma_helper.hpp"

#include <fstream>

nlohmann::json Helper::load_json(const std::string& json_file_name)
{
  std::ifstream json_fs(json_file_name);
  auto json_data = nlohmann::json::parse(json_fs);

  return json_data;
}
