#pragma once

#include <string>

namespace VGG
{
namespace StringHelper
{

std::string url_encode(const std::string& code);
std::string encode_script_to_data_uri(const std::string& code);

} // namespace StringHelper
}