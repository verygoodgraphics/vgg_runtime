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
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "JsonDocument.hpp"
#include "Loader.hpp"
#include "ModelEvent.hpp"
#include <nlohmann/json.hpp>
#include <rxcpp/rx-includes.hpp>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-predef.hpp>
#include <rxcpp/subjects/rx-subject.hpp>
namespace VGG
{
namespace Domain
{
class DesignDocument;
class Element;
} // namespace Domain
namespace Model
{
class Visitor;
namespace Detail
{
class DarumaImpl;
}
} // namespace Model
} // namespace VGG

namespace VGG
{

using MakeJsonDocFn = std::function<JsonDocumentPtr(const nlohmann::json&)>;

class Daruma
{
  std::unique_ptr<Model::Detail::DarumaImpl> m_impl;

  std::shared_ptr<VGG::Model::Loader> m_loader;

  std::unordered_map<std::string, std::string> m_memoryCode; // fileName: code_content
  nlohmann::json                               m_eventListeners;

  // original model
  JsonDocumentPtr              m_designDoc;
  MakeJsonDocFn                m_makeDesignDocFn;
  JsonDocumentPtr              m_layoutDoc;
  MakeJsonDocFn                m_makeLayoutDocFn;
  Model::Loader::ResourcesType m_resources;
  nlohmann::json               m_settingsDoc;

  // runtime view model, symbol instance expanded
  std::shared_ptr<Domain::DesignDocument> m_designDocTree;
  JsonDocumentPtr                         m_runtimeDesignDoc;
  JsonDocumentPtr                         m_runtimeLayoutDoc;

  rxcpp::subjects::subject<VGG::ModelEventPtr> m_subject;
  std::mutex                                   m_mutex;

public:
  using ListenersType = std::unordered_map<std::string, std::vector<std::string>>;

  Daruma(const MakeJsonDocFn& makeDesignDocFn, const MakeJsonDocFn& makeLayoutDocFn = {});
  ~Daruma();

  bool load(const std::string& path);   // zip file or dir
  bool load(std::vector<char>& buffer); // zip buffer

  void accept(VGG::Model::Visitor* visitor);

  JsonDocumentPtr runtimeDesignDoc();
  JsonDocumentPtr runtimeLayoutDoc();
  void            setRuntimeDesignDocTree(std::shared_ptr<Domain::DesignDocument> designDocTree);
  void            setRuntimeLayoutDoc(const nlohmann::json& layoutJson);

  JsonDocumentPtr& designDoc();
  JsonDocumentPtr& layoutDoc();
  auto             resources()
  {
    return m_resources;
  }

  std::string docVersion() const;

public:
  const std::string launchFrameId() const;
  bool              setLaunchFrameById(const std::string& id);

  const std::string currentTheme() const;
  bool              setCurrentTheme(const std::string& name);

  int getFrameIndexForWidth(double width) const; // -1: error

  int         getFrameIndexById(const std::string& id) const;
  std::string getFrameIdByIndex(const std::size_t index) const;

  std::unordered_set<std::string> texts() const;

public:
  bool updateElementFillColor(
    const std::string& id,
    const std::size_t  fillIndex,
    const double       r,
    const double       g,
    const double       b,
    const double       a);

public: // event listener
  void addEventListener(
    const std::string& targetKey, // id or json pointer path
    const std::string& type,
    const std::string& code);
  void removeEventListener(
    const std::string& targetKey,
    const std::string& type,
    const std::string& code);
  auto getEventListeners(const std::string& targetKey) -> ListenersType;

  // todo, make sure the wasm files (*.mjs *.wasm) are in the correct directory or url

public:
  // observable
  rxcpp::observable<VGG::ModelEventPtr> getObservable();

private:
  bool loadFiles();

  std::string getCode(const std::string& fileName);

  std::string uuidFor(const std::string& content);

  void getTextsTo(std::unordered_set<std::string>& texts, std::shared_ptr<Domain::Element> element)
    const;
};

} // namespace VGG
