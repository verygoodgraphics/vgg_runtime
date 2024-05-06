/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "DarumaImpl.hpp"
#include "JsonKeys.hpp"
#include "Loader/DirLoader.hpp"
#include "Loader/ZipLoader.hpp"
#include "SubjectJsonDocument.hpp"
#include "RawJsonDocument.hpp"

#include "Domain/Model/DesignDocAdapter.hpp"
#include "Domain/Model/Element.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>

using namespace VGG;
using namespace VGG::Model;
namespace fs = std::filesystem;

Daruma::Daruma(const MakeJsonDocFn& makeDesignDocFn, const MakeJsonDocFn& makeLayoutDocFn)
  : m_impl{ new Model::Detail::DarumaImpl }
  , m_makeDesignDocFn{ makeDesignDocFn }
  , m_makeLayoutDocFn{ makeLayoutDocFn }
{
}

Daruma::~Daruma() = default;

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
  const std::lock_guard<std::mutex> lock(m_mutex);

  visitor->visit(K_DESIGN_FILE_NAME, m_designDoc->content().dump());
  visitor->visit(K_EVENT_LISTENERS_FILE_NAME, m_eventListeners.dump());
  if (m_layoutDoc && m_layoutDoc->content().is_object())
  {
    visitor->visit(K_LAYOUT_FILE_NAME, m_layoutDoc->content().dump());
  }
  if (m_settingsDoc.is_object())
  {
    nlohmann::json j = m_impl->settings();
    visitor->visit(K_SETTINGS_FILE_NAME, j.dump());
  }

  // js
  for (auto& [path, elementEventListeners] : m_eventListeners.items())
  {
    for (auto& [type, typeEventListeners] : elementEventListeners.items())
    {
      std::vector<std::string> listeners{};
      for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
      {
        if (it->is_object() && it->contains(K_FILE_NAME_KEY))
        {
          auto& fileName = (*it)[K_FILE_NAME_KEY];
          visitor->visit(fileName, getCode(fileName));
        }
      }
    }
  }

  // resouces
  auto        resouces = m_loader->resources();
  std::string resoucesDir{ Model::K_RESOURCES_DIR_WITH_SLASH };

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
    if (m_loader->readFile(K_DESIGN_FILE_NAME, fileContent))
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

    if (m_loader->readFile(K_LAYOUT_FILE_NAME, fileContent) && m_makeLayoutDocFn)
    {
      auto tmpJson = json::parse(fileContent);
      auto doc = m_makeLayoutDocFn(tmpJson);
      m_layoutDoc = JsonDocumentPtr(new SubjectJsonDocument(doc));
      m_runtimeLayoutDoc = m_layoutDoc;
    }
    else
    {
      DEBUG("#Daruma::loadFiles(), read layout file failed");
    }

    if (m_loader->readFile(K_SETTINGS_FILE_NAME, fileContent))
    {
      m_settingsDoc = json::parse(fileContent);
      m_impl->setSettings(m_settingsDoc);
    }
    else
    {
      DEBUG("#Daruma::loadFiles(), read settings file failed");
    }

    if (m_loader->readFile(K_EVENT_LISTENERS_FILE_NAME, fileContent))
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

void Daruma::setRuntimeDesignDocTree(std::shared_ptr<Domain::DesignDocument> designDocTree)
{
  ASSERT(designDocTree);

  m_designDocTree = designDocTree;
  auto doc = std::make_shared<DesignDocAdapter>(m_designDocTree);
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

void Daruma::addEventListener(
  const std::string& targetKey,
  const std::string& type,
  const std::string& code)
{
  const std::lock_guard<std::mutex> lock(m_mutex);

  // generate file name
  auto fileName = uuidFor(code) + K_JS_FILE_SUFFIX;

  // create element listenters object
  if (!m_eventListeners.contains(targetKey))
  {
    m_eventListeners[targetKey] = json(json::value_t::object);
  }

  // create element listenters array for `type`
  auto& elementEventListeners = m_eventListeners[targetKey];
  if (!elementEventListeners.contains(type))
  {
    elementEventListeners[type] = json(json::value_t::array);
  }

  // return if exist
  auto& typeEventListeners = elementEventListeners[type];
  for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
  {
    if (it->is_object() && it->contains(K_FILE_NAME_KEY))
    {
      auto& itemFileName = (*it)[K_FILE_NAME_KEY];
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
  item[K_FILE_NAME_KEY] = fileName;
  using namespace std::chrono;
  item[K_CREATED_AT_KEY] =
    duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  // save meta info
  typeEventListeners.push_back(item);

  // todo, notify observer?
  // todo, edit mode, save code & meta to remote server
}

void Daruma::removeEventListener(
  const std::string& targetKey,
  const std::string& type,
  const std::string& code)
{
  if (!m_eventListeners.contains(targetKey))
  {
    return;
  }

  auto& elementEventListeners = m_eventListeners[targetKey];
  if (!elementEventListeners.contains(type))
  {
    return;
  }

  auto fileName = uuidFor(code) + K_JS_FILE_SUFFIX;

  auto& typeEventListeners = elementEventListeners[type];
  for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
  {
    if (it->is_object() && it->contains(K_FILE_NAME_KEY))
    {
      auto& itemFileName = (*it)[K_FILE_NAME_KEY];
      if (itemFileName.is_string() && itemFileName.get<std::string>() == fileName)
      {
        const std::lock_guard<std::mutex> lock(m_mutex);

        typeEventListeners.erase(it);

        // todo, notify observer?
        return;
      }
    }
  }
}

auto Daruma::getEventListeners(const std::string& targetKey) -> ListenersType
{
  const std::lock_guard<std::mutex> lock(m_mutex);

  ListenersType result{};

  if (targetKey.empty() || !m_eventListeners.contains(targetKey))
  {
    return result;
  }

  auto& elementEventListeners = m_eventListeners[targetKey];
  for (auto& [type, typeEventListeners] : elementEventListeners.items())
  {
    std::vector<std::string> listeners{};
    for (auto it = typeEventListeners.cbegin(); it != typeEventListeners.cend(); ++it)
    {
      if (it->is_object() && it->contains(K_FILE_NAME_KEY))
      {
        listeners.push_back(getCode((*it)[K_FILE_NAME_KEY]));
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

std::string Daruma::docVersion() const
{
  ASSERT(m_designDocTree);

  return m_designDocTree->designModel()->version;
}

std::string Daruma::getFrameIdByIndex(const std::size_t index) const
{
  ASSERT(m_designDocTree);

  if (index < m_designDocTree->children().size())
  {
    return m_designDocTree->children()[index]->id();
  }

  return {};
}

int Daruma::getFrameIndexById(const std::string& id) const
{
  ASSERT(m_designDocTree);

  for (std::size_t i = 0; i < m_designDocTree->children().size(); ++i)
  {
    if (m_designDocTree->children()[i]->id() == id)
    {
      return i;
    }
  }

  return -1;
}

std::unordered_set<std::string> Daruma::texts() const
{
  ASSERT(m_designDocTree);

  std::unordered_set<std::string> texts;
  getTextsTo(texts, m_designDocTree);

  return texts;
}

const std::string Daruma::launchFrameId() const
{
  return m_impl->settings().launchFrameId;
}

bool Daruma::setLaunchFrameById(const std::string& id)
{
  if (getFrameIndexById(id) >= 0)
  {
    m_impl->setLaunchFrameById(id);
    return true;
  }

  return false;
}

void Daruma::getTextsTo(
  std::unordered_set<std::string>& texts,
  std::shared_ptr<Domain::Element> element) const
{
  if (auto textElement = std::dynamic_pointer_cast<Domain::TextElement>(element))
  {
    if (auto& content = textElement->object()->content; !content.empty())
    {
      texts.insert(content);
    }
  }

  for (auto& child : element->children())
  {
    getTextsTo(texts, child);
  }
}

int Daruma::getFrameIndexForWidth(double width) const
{
  return getFrameIndexById(m_impl->frameIdForWidth(width));
}

const std::string Daruma::currentTheme() const
{
  return m_impl->currentTheme();
}

bool Daruma::setCurrentTheme(const std::string& name)
{
  return m_impl->setCurrentTheme(name);
}

bool Daruma::updateElementFillColor(
  const std::string& id,
  const std::size_t  fillIndex,
  const double       r,
  const double       g,
  const double       b,
  const double       a)
{
  if (!m_designDocTree)
    return false;

  auto element = m_designDocTree->getElementByKey(id);
  if (!element)
    return false;

  auto object = element->object();
  if (!object)
    return false;

  if (fillIndex >= object->style.fills.size())
    return false;

  Model::Color c;
  c.alpha = a;
  c.red = r;
  c.green = g;
  c.blue = b;
  c.colorClass = Model::BackgroundColorClass::COLOR;
  object->style.fills[fillIndex].color = c;

  return true;
}