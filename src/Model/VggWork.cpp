#include "VggWork.hpp"

#include "Utils/Utils.hpp"

#include "zip.h"

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>

constexpr auto artboard_file_name = "artboard.json";
constexpr auto code_map_file_name = "code_map.json";
constexpr auto event_listeners_file_name = "event_listeners.json";
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

    if (readZipFileEntry(zipFile, event_listeners_file_name, file_content))
    {
      m_event_listeners = json::parse(file_content);
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

void VggWork::addEventListener(const std::string& json_pointer,
                               const std::string& type,
                               const std::string& code)
{
  // generate file name
  auto file_name = uuid_for(code) + ".mjs";

  // fill empty object/array
  if (m_event_listeners.find(json_pointer) == m_event_listeners.end())
  {
    m_event_listeners[json_pointer] = json(json::value_t::object);
  }
  auto& element_event_listeners = m_event_listeners[json_pointer];
  if (element_event_listeners.find(type) == element_event_listeners.end())
  {
    element_event_listeners[type] = json(json::value_t::array);
  }

  // return if exist
  auto& type_event_listeners = element_event_listeners[type];
  for (auto it = type_event_listeners.cbegin(); it != type_event_listeners.cend(); ++it)
  {
    if (it->is_object())
    {
      auto file_name_it = it->find("fileName");
      if (file_name_it != it->cend())
      {
        if (file_name_it->is_string() && file_name_it->get<std::string>() == file_name)
        {
          return;
        }
      }
    }
  }

  // save code content
  m_memory_code[file_name] = code;

  // fill meta info
  auto item = json(json::value_t::object);
  item["fileName"] = file_name;
  using namespace std::chrono;
  item["createdAt"] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  // save meta info
  type_event_listeners.push_back(item);

  // todo, save code & meta to remote server
}

void VggWork::removeEventListener(const std::string& json_pointer,
                                  const std::string& type,
                                  const std::string& code)
{
}

const std::vector<std::string> VggWork::getEventListeners(const std::string& json_pointer,
                                                          const std::string& type)
{
  return {};
}

std::string VggWork::uuid_for(const std::string& content)
{
  boost::uuids::name_generator_sha1 generator{ boost::uuids::ns::oid() };
  return boost::uuids::to_string(generator(content));
}