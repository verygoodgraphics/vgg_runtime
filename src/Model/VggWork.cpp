#include "VggWork.hpp"

#include "Config.hpp"
#include "Loader/DirLoader.hpp"
#include "Loader/ZipLoader.hpp"
#include "SubjectJsonDocument.hpp"
#include "Utils/Utils.hpp"

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>

using namespace VGG;
namespace fs = std::filesystem;

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

bool VggWork::load(const std::string& path)
{
  if (fs::is_regular_file(path))
  {
    m_loader.reset(new Model::ZipLoader(path));
  }
  else if (fs::is_directory(path))
  {
    m_loader.reset(new Model::DirLoader(path));
  }
  else
  {
    return false;
  }

  return load_files();
}

bool VggWork::load(std::vector<char>& buffer)
{
  m_loader.reset(new Model::ZipLoader(buffer));
  return load_files();
}

void VggWork::visit(VGG::Model::Visitor* visitor)
{
  // todo, lock for thread safe
  const std::lock_guard<std::mutex> lock(m_mutex);

  visitor->accept(artboard_file_name, m_designDoc->content().dump());
  visitor->accept(event_listeners_file_name, m_event_listeners.dump());

  // js
  for (auto& [path, element_event_listeners] : m_event_listeners.items())
  {
    for (auto& [type, type_event_listeners] : element_event_listeners.items())
    {
      std::vector<std::string> listeners{};
      for (auto it = type_event_listeners.cbegin(); it != type_event_listeners.cend(); ++it)
      {
        if (it->is_object() && it->contains(file_name_key))
        {
          auto& file_name = (*it)[file_name_key];
          visitor->accept(file_name, get_code(file_name));
        }
      }
    }
  }

  // resouces
  auto resouces = m_loader->resources();
  std::string resouces_dir{ Model::ResourcesDirWithSlash };

  for (auto& [name, content] : resouces)
  {
    visitor->accept(resouces_dir + name, content);
  }

  // todo, other files
}

bool VggWork::load_files()
{
  try
  {
    std::string file_content;
    if (m_loader->readFile(artboard_file_name, file_content))
    {
      auto tmp_json = json::parse(file_content);
      auto doc = m_makeDesignDocFn(tmp_json);
      m_designDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
    }
    else
    {
      return false;
    }

    if (m_loader->readFile(event_listeners_file_name, file_content))
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
  m_loader->readFile(file_name, code);
  return code;
}

std::string VggWork::uuid_for(const std::string& content)
{
  boost::uuids::name_generator_sha1 generator{ boost::uuids::ns::oid() };
  return boost::uuids::to_string(generator(content));
}