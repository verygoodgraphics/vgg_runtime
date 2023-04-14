#include "VggWork.hpp"

#include "Utils/Utils.hpp"

#include <miniz-cpp/zip_file.hpp>

constexpr auto artboard_file_name = "artboard.json";
constexpr auto code_map_file_name = "code_map.json";
constexpr auto layout_file_name = "layout.json";

VggWork::VggWork(const MakeJsonDocFn& makeDesignDocFn)
  : m_makeDesignDocFn(makeDesignDocFn)
{
}

bool VggWork::load(const std::string& filePath)
{
  return loadTemplate([&](miniz_cpp::zip_file& file) { file.load(filePath); });
}

bool VggWork::load(const std::vector<unsigned char>& buffer)
{
  return loadTemplate([&](miniz_cpp::zip_file& file) { file.load(buffer); });
}

bool VggWork::loadTemplate(LoadZipFn fn)
{
  miniz_cpp::zip_file zip_file;

  try
  {
    fn(zip_file);
    return load(zip_file);
  }
  catch (const std::runtime_error& err)
  {
    FAIL("Failed to load vgg file: %s", err.what());
    return false;
  }
}

bool VggWork::load(miniz_cpp::zip_file& zipFile)
{
  try
  {
    for (auto entry : zipFile.namelist())
    {
      if (entry.rfind(artboard_file_name, 0) == 0)
      {
        auto json = json::parse(zipFile.read(entry));
        m_designDoc = m_makeDesignDocFn(json);

        return true;
      }
    }
    return false;
  }
  catch (const std::exception& e)
  {
    return false;
  }
}

const json& VggWork::designDoc() const
{
  return m_designDoc->content();
}