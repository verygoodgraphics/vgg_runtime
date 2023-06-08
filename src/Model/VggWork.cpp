#include "VggWork.hpp"

#include "Model/SubjectJsonDocument.hpp"
#include "Utils/Utils.hpp"

#include "zip.h"

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>

using namespace VGG;

constexpr auto artboard_file_name = "artboard.json";
constexpr auto event_listeners_file_name = "event_listeners.json";
constexpr auto layout_file_name = "layout.json";

constexpr auto file_name_key = "fileName";
constexpr auto created_at_key = "createdAt";

constexpr auto js_file_suffix = ".mjs";

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

bool VggWork::load(std::vector<char>& buffer)
{
  m_zip_buffer = std::move(buffer);
  m_zipFile = zip_stream_open(m_zip_buffer.data(), m_zip_buffer.size(), 0, 'r');
  return load(m_zipFile);
}

bool VggWork::load(zip_t* zipFile)
{
  try
  {
    std::string file_content;
    if (readZipFileEntry(zipFile, artboard_file_name, file_content))
    {
      auto tmp_json = json::parse(file_content);
      auto doc = m_makeDesignDocFn(tmp_json);
      m_designDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
    }
    else
    {
      return false;
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
  const std::lock_guard<std::mutex> lock(m_mutex);

  // generate file name
  auto file_name = uuid_for(code) + js_file_suffix;

  // create element listenters object
  if (!m_event_listeners.contains(json_pointer))
  {
    m_event_listeners[json_pointer] = json(json::value_t::object);
  }

  // create element listenters array for `type`
  auto& element_event_listeners = m_event_listeners[json_pointer];
  if (!element_event_listeners.contains(type))
  {
    element_event_listeners[type] = json(json::value_t::array);
  }

  // return if exist
  auto& type_event_listeners = element_event_listeners[type];
  for (auto it = type_event_listeners.cbegin(); it != type_event_listeners.cend(); ++it)
  {
    if (it->is_object() && it->contains(file_name_key))
    {
      auto& item_file_name = (*it)[file_name_key];
      if (item_file_name.is_string() && item_file_name.get<std::string>() == file_name)
      {
        return;
      }
    }
  }

  // save code content
  m_memory_code[file_name] = code;

  // fill meta info
  auto item = json(json::value_t::object);
  item[file_name_key] = file_name;
  using namespace std::chrono;
  item[created_at_key] =
    duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  // save meta info
  type_event_listeners.push_back(item);

  m_subject.get_subscriber().on_next(
    ModelEventPtr{ new ModelEventListenerDidAdd{ json::json_pointer(json_pointer) } });

  // todo, edit mode, save code & meta to remote server
}

void VggWork::removeEventListener(const std::string& json_pointer,
                                  const std::string& type,
                                  const std::string& code)
{
  if (!m_event_listeners.contains(json_pointer))
  {
    return;
  }

  auto& element_event_listeners = m_event_listeners[json_pointer];
  if (!element_event_listeners.contains(type))
  {
    return;
  }

  auto file_name = uuid_for(code) + js_file_suffix;

  auto& type_event_listeners = element_event_listeners[type];
  for (auto it = type_event_listeners.cbegin(); it != type_event_listeners.cend(); ++it)
  {
    if (it->is_object() && it->contains(file_name_key))
    {
      auto& item_file_name = (*it)[file_name_key];
      if (item_file_name.is_string() && item_file_name.get<std::string>() == file_name)
      {
        const std::lock_guard<std::mutex> lock(m_mutex);

        type_event_listeners.erase(it);

        m_subject.get_subscriber().on_next(
          ModelEventPtr{ new ModelEventListenerDidRemove{ json::json_pointer(json_pointer) } });
        return;
      }
    }
  }
}

auto VggWork::getEventListeners(const std::string& json_pointer) -> ListenersType
{
  const std::lock_guard<std::mutex> lock(m_mutex);

  ListenersType result{};

  if (!m_event_listeners.contains(json_pointer))
  {
    return result;
  }

  auto& element_event_listeners = m_event_listeners[json_pointer];
  for (auto& [type, type_event_listeners] : element_event_listeners.items())
  {
    std::vector<std::string> listeners{};
    for (auto it = type_event_listeners.cbegin(); it != type_event_listeners.cend(); ++it)
    {
      if (it->is_object() && it->contains(file_name_key))
      {
        listeners.push_back(get_code((*it)[file_name_key]));
      }
    }
    result[type] = listeners;
  }

  return result;
}

// observable
rxcpp::observable<VGG::ModelEventPtr> VggWork::getObservable()
{
  auto result = m_subject.get_observable();

  auto doc = m_designDoc.get();
  if (auto sub_doc = dynamic_cast<SubjectJsonDocument*>(doc))
  {
    result = result.merge(sub_doc->getObservable());
  }

  return result;
}

std::string VggWork::get_code(const std::string& file_name)
{
  if (auto it = m_memory_code.find(file_name); it != m_memory_code.end())
  {
    return m_memory_code[file_name];
  }

  std::string code;
  readZipFileEntry(m_zipFile, file_name, code);
  return code;
}

std::string VggWork::uuid_for(const std::string& content)
{
  boost::uuids::name_generator_sha1 generator{ boost::uuids::ns::oid() };
  return boost::uuids::to_string(generator(content));
}