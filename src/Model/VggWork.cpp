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
  m_zipFile.reset(new miniz_cpp::zip_file);

  try
  {
    fn(*m_zipFile);
    return load(*m_zipFile);
  }
  catch (const std::runtime_error& err)
  {
    FAIL("Failed to load vgg file: %s", err.what());
    return false;
  }
}

const std::string VggWork::getCode(const std::string& path) const
{
  auto name = m_codeMap.at(path);
  std::string code;
  readZipFileEntry(*m_zipFile, name, code);
  // todo, cache js file?
  return code;
}

bool VggWork::load(miniz_cpp::zip_file& zipFile)
{
  try
  {
    std::string file_content;
    if (readZipFileEntry(zipFile, artboard_file_name, file_content))
    {
      auto tmp_json = json::parse(file_content);
      m_designDoc = m_makeDesignDocFn(tmp_json);
    }
    else
    {
      return false;
    }

    if (readZipFileEntry(zipFile, code_map_file_name, file_content))
    {
      auto tmp_json = json::parse(file_content);
      tmp_json.get_to(m_codeMap);
    }

    return true;
  }
  catch (const std::exception& e)
  {
    return false;
  }
}

bool VggWork::readZipFileEntry(miniz_cpp::zip_file& zipFile,
                               const std::string& entryName,
                               std::string& content) const
{
  for (auto entry : zipFile.namelist())
  {
    if (entry.rfind(entryName, 0) == 0)
    {
      content = zipFile.read(entry);
      return true;
    }
  }

  return false;
}

JsonDocumentPtr& VggWork::designDoc()
{
  return m_designDoc;
}