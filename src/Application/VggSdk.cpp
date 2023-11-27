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
#include "VggSdk.hpp"

#include "Domain/DarumaContainer.hpp"
#include "Domain/IVggEnv.hpp"
#include "Domain/Saver/DirSaver.hpp"
#include "UseCase/SaveModel.hpp"
#include "Utility/DIContainer.hpp"
#include "Utility/Log.hpp"

#ifdef EMSCRIPTEN
constexpr auto listener_code_key = "listener";
#endif

using namespace VGG;

namespace
{
auto env()
{
  return VGG::DIContainer<std::shared_ptr<IVggEnv>>::get();
}
} // namespace

std::string VggSdk::getEnvKey()
{
  return env()->getEnv();
}
void VggSdk::setContainerKey(const std::string& containerKey)
{
  env()->setContainerKey(containerKey);
}
void VggSdk::setInstanceKey(const std::string& instanceKey)
{
  env()->setInstanceKey(instanceKey);
}
void VggSdk::setListenerKey(const std::string& listenerKey)
{
  env()->setListenerKey(listenerKey);
}

// design document in vgg daruma file
const std::string VggSdk::designDocument()
{
  return getDesignDocument()->content().dump();
}

void VggSdk::designDocumentReplaceAt(const std::string& jsonPointer, const std::string& value)
{
  getDesignDocument()->replaceAt(jsonPointer, value);
}

void VggSdk::designDocumentAddAt(const std::string& jsonPointer, const std::string& value)
{
  getDesignDocument()->addAt(jsonPointer, value);
}

void VggSdk::designDocumentDeleteAt(const std::string& jsonPointer)
{
  getDesignDocument()->deleteAt(jsonPointer);
}

// event listener
void VggSdk::addEventListener(
  const std::string& elementPath,
  const std::string& eventType,
  const std::string& listenerCode)
{
  getModel()->addEventListener(elementPath, eventType, listenerCode);
}

void VggSdk::removeEventListener(
  const std::string& elementPath,
  const std::string& eventType,
  const std::string& listenerCode)
{
  getModel()->removeEventListener(elementPath, eventType, listenerCode);
}

VggSdk::ListenersType VggSdk::getEventListeners(const std::string& elementPath)
{
#ifdef EMSCRIPTEN
  using namespace emscripten;

  auto result_listeners_map = val::object();
  auto listenersMap = getModel()->getEventListeners(elementPath);
  for (auto& map_item : listenersMap)
  {
    if (map_item.second.empty())
    {
      continue;
    }

    auto& eventType = map_item.first;

    auto js_listener_code_array = val::array();
    for (int i = 0; i < map_item.second.size(); ++i)
    {
      auto& listenerCode = map_item.second[i];
      auto  js_listener_object = val::object();

      js_listener_object.set(listener_code_key, val(listenerCode.c_str()));
      js_listener_code_array.set(i, js_listener_object);
    }

    result_listeners_map.set(eventType.c_str(), js_listener_code_array);
  }

  return result_listeners_map;
#else
  return getModel()->getEventListeners(elementPath);
#endif
}

// vgg model
std::shared_ptr<JsonDocument> VggSdk::getDesignDocument(IndexType index)
{
  return getModel(index)->runtimeDesignDoc();
}
std::shared_ptr<Daruma> VggSdk::getModel(IndexType index)
{
  auto key = index == main_or_editor_daruma_index ? DarumaContainer::KeyType::MainOrEditor
                                                  : DarumaContainer::KeyType::Edited;
  return DarumaContainer().get(key);
}

void VggSdk::save()
{
  auto editModel = getModel(edited_daruma_index);
  if (editModel)
  {
    // todo, choose location to save, or save to remote server
    std::string dstDir{ "tmp/" };
    auto        saver{ std::make_shared<Model::DirSaver>(dstDir) };
    SaveModel   saveModel{ editModel, saver };

    saveModel.save();
  }
  else
  {
    WARN("VggSdk: no daruma file to save");
  }
}
