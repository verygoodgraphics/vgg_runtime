#include "BrowserJSEngine.hpp"

#include <emscripten/emscripten.h>

#include <cctype>
#include <iomanip>
#include <sstream>

bool BrowserJSEngine::evalScript(const std::string& code)
{
  emscripten_run_script(code.c_str());
  return true;
}

bool BrowserJSEngine::evalModule(const std::string& code)
{
  // https://2ality.com/2019/10/eval-via-import.html
  auto code_before = "const dataUri = 'data:text/javascript;charset=utf-8,' + '";
  auto code_after = "'; import(dataUri);";

  m_moduleWrapper.erase();
  m_moduleWrapper.append(code_before);
  m_moduleWrapper.append(url_encode(code));
  m_moduleWrapper.append(code_after);

  emscripten_run_script(m_moduleWrapper.c_str());
  return true;
}

std::string BrowserJSEngine::url_encode(const std::string& value)
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