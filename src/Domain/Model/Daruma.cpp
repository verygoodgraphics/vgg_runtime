/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Daruma.hpp"

#include "Config.hpp"
#include "Loader/DirLoader.hpp"
#include "Loader/ZipLoader.hpp"
#include "SubjectJsonDocument.hpp"
#include "Utility/Log.hpp"

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>

using namespace VGG;
using namespace VGG::Model;
namespace fs = std::filesystem;

Daruma::Daruma(const MakeJsonDocFn& makeDesignDocFn, const MakeJsonDocFn& makeLayoutDocFn)
  : m_makeDesignDocFn{ makeDesignDocFn }
  , m_makeLayoutDocFn{ makeLayoutDocFn }
{
}

bool Daruma::load(const std::string& path)
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
    WARN("Daruma::load, invaid file path, %s, return", path.c_str());
    return false;
  }

  return loadFiles();
}

bool Daruma::load(std::vector<char>& buffer)
{
  m_loader.reset(new Model::ZipLoader(buffer));
  return loadFiles();
}

void Daruma::accept(VGG::Model::Visitor* visitor)
{
  // todo, lock for thread safe
  const std::lock_guard<std::mutex> lock(m_mutex);

  visitor->visit(design_file_name, m_designDoc->content().dump());
  visitor->visit(event_listeners_file_name, m_eventListeners.dump());
  if (m_layoutDoc && m_layoutDoc->content().is_object())
  {
    visitor->visit(layout_file_name, m_layoutDoc->content().dump());
  }

  // js
  for (auto& [path, elementEventListeners] : m_eventListeners.items())
  {
    for (auto& [type, typeEventListeners] : elementEventListeners.items())
    {
      std::vector<std::string> listeners{};
      for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
      {
        if (it->is_object() && it->contains(file_name_key))
        {
          auto& fileName = (*it)[file_name_key];
          visitor->visit(fileName, getCode(fileName));
        }
      }
    }
  }

  // resouces
  auto resouces = m_loader->resources();
  std::string resoucesDir{ Model::ResourcesDirWithSlash };

  for (auto& [name, content] : resouces)
  {
    visitor->visit(resoucesDir + name, content);
  }
}

bool Daruma::loadFiles()
{
  try
  {
    std::string fileContent;
    if (m_loader->readFile(design_file_name, fileContent))
    {
      auto tmpJson = json::parse(fileContent);
      auto doc = m_makeDesignDocFn(tmpJson);
      m_designDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
      m_runtimeDesignDoc = m_designDoc;
    }
    else
    {
      FAIL("#Daruma::loadFiles(), read file failed");
      return false;
    }

    if (m_loader->readFile(layout_file_name, fileContent) && m_makeLayoutDocFn)
    {
      auto tmpJson = json::parse(fileContent);
      auto doc = m_makeLayoutDocFn(tmpJson);
      m_layoutDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
      m_runtimeLayoutDoc = m_layoutDoc;
    }
    else
    {
      WARN("#Daruma::loadFiles(), read layout file failed");
    }

    if (m_loader->readFile(event_listeners_file_name, fileContent))
    {
      m_eventListeners = json::parse(fileContent);
    }
    else
    {
      m_eventListeners = json::object();
    }

    m_resources = m_loader->resources();

    return true;
  }
  catch (const std::exception& e)
  {
    FAIL("#Daruma::loadFiles(), caught exception: %s", e.what());
    return false;
  }
}

JsonDocumentPtr Daruma::runtimeDesignDoc()
{
  return m_runtimeDesignDoc;
}

JsonDocumentPtr Daruma::runtimeLayoutDoc()
{
  return m_runtimeLayoutDoc;
}

void Daruma::setRuntimeDesignDoc(const nlohmann::json& designJson)
{
  auto doc = m_makeDesignDocFn(designJson);
  m_runtimeDesignDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
}

void Daruma::setRuntimeLayoutDoc(const nlohmann::json& layoutJson)
{
  auto doc = m_makeLayoutDocFn(layoutJson);
  m_runtimeLayoutDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
}

JsonDocumentPtr& Daruma::designDoc()
{
  return m_designDoc;
}

JsonDocumentPtr& Daruma::layoutDoc()
{
  return m_layoutDoc;
}

void Daruma::addEventListener(const std::string& jsonPointer,
                              const std::string& type,
                              const std::string& code)
{
  const std::lock_guard<std::mutex> lock(m_mutex);

  // generate file name
  auto fileName = uuidFor(code) + js_file_suffix;

  // create element listenters object
  if (!m_eventListeners.contains(jsonPointer))
  {
    m_eventListeners[jsonPointer] = json(json::value_t::object);
  }

  // create element listenters array for `type`
  auto& elementEventListeners = m_eventListeners[jsonPointer];
  if (!elementEventListeners.contains(type))
  {
    elementEventListeners[type] = json(json::value_t::array);
  }

  // return if exist
  auto& typeEventListeners = elementEventListeners[type];
  for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
  {
    if (it->is_object() && it->contains(file_name_key))
    {
      auto& itemFileName = (*it)[file_name_key];
      if (itemFileName.is_string() && itemFileName.get<std::string>() == fileName)
      {
        return;
      }
    }
  }

  // save code content
  m_memoryCode[fileName] = code;

  // fill meta info
  auto item = json(json::value_t::object);
  item[file_name_key] = fileName;
  using namespace std::chrono;
  item[created_at_key] =
    duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  // save meta info
  typeEventListeners.push_back(item);

  m_subject.get_subscriber().on_next(
    ModelEventPtr{ new ModelEventListenerDidAdd{ json::json_pointer(jsonPointer) } });

  // todo, edit mode, save code & meta to remote server
}

void Daruma::removeEventListener(const std::string& jsonPointer,
                                 const std::string& type,
                                 const std::string& code)
{
  if (!m_eventListeners.contains(jsonPointer))
  {
    return;
  }

  auto& elementEventListeners = m_eventListeners[jsonPointer];
  if (!elementEventListeners.contains(type))
  {
    return;
  }

  auto fileName = uuidFor(code) + js_file_suffix;

  auto& typeEventListeners = elementEventListeners[type];
  for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
  {
    if (it->is_object() && it->contains(file_name_key))
    {
      auto& itemFileName = (*it)[file_name_key];
      if (itemFileName.is_string() && itemFileName.get<std::string>() == fileName)
      {
        const std::lock_guard<std::mutex> lock(m_mutex);

        typeEventListeners.erase(it);

        m_subject.get_subscriber().on_next(
          ModelEventPtr{ new ModelEventListenerDidRemove{ json::json_pointer(jsonPointer) } });
        return;
      }
    }
  }
}

auto Daruma::getEventListeners(const std::string& jsonPointer) -> ListenersType
{
  const std::lock_guard<std::mutex> lock(m_mutex);

  ListenersType result{};

  if (!m_eventListeners.contains(jsonPointer))
  {
    return result;
  }

  auto& elementEventListeners = m_eventListeners[jsonPointer];
  for (auto& [type, typeEventListeners] : elementEventListeners.items())
  {
    std::vector<std::string> listeners{};
    for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
    {
      if (it->is_object() && it->contains(file_name_key))
      {
        listeners.push_back(getCode((*it)[file_name_key]));
      }
    }
    result[type] = listeners;
  }

  return result;
}

// observable
rxcpp::observable<VGG::ModelEventPtr> Daruma::getObservable()
{
  auto result = m_subject.get_observable();

  auto doc = m_runtimeDesignDoc.get();
  if (auto subjectDoc = dynamic_cast<SubjectJsonDocument*>(doc))
  {
    result = result.merge(subjectDoc->getObservable());
  }

  return result;
}

std::string Daruma::getCode(const std::string& fileName)
{
  if (auto it = m_memoryCode.find(fileName); it != m_memoryCode.end())
  {
    return m_memoryCode[fileName];
  }

  std::string code;
  m_loader->readFile(fileName, code);
  return code;
}

std::string Daruma::uuidFor(const std::string& content)
{
  boost::uuids::name_generator_sha1 generator{ boost::uuids::ns::oid() };
  return boost::uuids::to_string(generator(content));
}
