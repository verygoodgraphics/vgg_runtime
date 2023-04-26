#include "VggWork.hpp"

#include "Utils/Utils.hpp"

#include "zip.h"

constexpr auto artboard_file_name = "artboard.json";
constexpr auto code_map_file_name = "code_map.json";
constexpr auto layout_file_name = "layout.json";

VggWork::VggWork(const MakeJsonDocFn& makeDesignDocFn)
  : m_makeDesignDocFn(makeDesignDocFn)
{
}

VggWork::~VggWork()
{
  if (m_zipFile)
  {
    zip_close(m_zipFile);
    m_zipFile = nullptr;
  }
}

bool VggWork::load(const std::string& filePath)
{
  m_zipFile = zip_open(filePath.c_str(), 0, 'r');
  return load(m_zipFile);
}

bool VggWork::load(const std::vector<char>& buffer)
{
  m_zipFile = zip_stream_open(buffer.data(), buffer.size(), 0, 'r');
  return load(m_zipFile);
}

const std::string VggWork::getCode(const std::string& path) const
{
  auto name = m_codeMap.at(path);
  std::string code;
  readZipFileEntry(m_zipFile, name, code);
  // todo, cache js file?
  return code;
}

bool VggWork::load(zip_t* zipFile)
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

bool VggWork::readZipFileEntry(zip_t* zipFile,
                               const std::string& entryName,
                               std::string& content) const
{
  content.clear();

  if (0 == zip_entry_open(m_zipFile, entryName.c_str()))
  {
    void* buf = NULL;
    size_t bufsize;
    zip_entry_read(m_zipFile, &buf, &bufsize);
    zip_entry_close(m_zipFile);

    content.append(static_cast<char*>(buf), bufsize);
    free(buf);

    return true;
  }

  return false;
}

JsonDocumentPtr& VggWork::designDoc()
{
  return m_designDoc;
}