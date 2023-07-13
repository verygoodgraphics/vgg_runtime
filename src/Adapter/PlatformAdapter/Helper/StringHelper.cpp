#include "PlatformAdapter/Helper/StringHelper.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace VGG
{

namespace StringHelper
{

std::string encode_script_to_data_uri(const std::string& code)
{
  // https://2ality.com/2019/10/eval-via-import.html

  std::string result("'data:text/javascript;charset=utf-8,");
  result.append(url_encode(code));
  result.append("'");
  return result;
}

std::string url_encode(const std::string& value)
{
  using namespace std;

  ostringstream escaped;
  escaped.fill('0');
  escaped << hex;

  for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
  {
    string::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << uppercase;
    escaped << '%' << setw(2) << int((unsigned char)c);
    escaped << nouppercase;
  }

  return escaped.str();
}

} // namespace StringHelper
} // namespace VGG
