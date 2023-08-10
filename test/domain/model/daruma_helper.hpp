#include "Domain/RawJsonDocument.hpp"

namespace Helper
{

auto RawJsonDocumentBuilder = [](const json& design_json)
{
  auto raw_json_doc = new RawJsonDocument();
  raw_json_doc->setContent(design_json);
  return JsonDocumentPtr(raw_json_doc);
};

nlohmann::json load_json(const std::string& json_file_name);

} // namespace Helper