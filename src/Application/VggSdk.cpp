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

#include "Application/Presenter.hpp"
#include "Application/VggEnv.hpp"
#include "Domain/IVggEnv.hpp"
#include "Domain/Saver/DirSaver.hpp"
#include "Domain/Saver/ZipStreamSaver.hpp"
#include "Layer/FontManager.hpp"
#include "UseCase/SaveModel.hpp"
#include "Utility/Log.hpp"

#ifdef EMSCRIPTEN
constexpr auto listener_code_key = "listener";
#endif

using namespace VGG;

std::shared_ptr<VGG::VggEnv> VggSdk::env() const
{
  if (auto env = m_env.lock())
  {
    return env;
  }
  else
  {
    return VggEnv::getDefault().lock();
  }
}

void VggSdk::setEnv(const std::string& key)
{
  m_env = VggEnv::get(key);
}

std::string VggSdk::getEnv()
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
std::string VggSdk::designDocument()
{
  return getDesignDocument()->content().dump();
}

std::string VggSdk::designDocumentValueAt(const std::string& jsonPointer)
{
  const auto&                  docJson = getDesignDocument()->content();
  nlohmann::json::json_pointer path{ jsonPointer };
  return docJson.at(path).dump();
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

std::string VggSdk::getElement(const std::string& id)
{
  return getDesignDocument()->getElement(id);
}

void VggSdk::updateElement(const std::string& id, const std::string& contentJsonString)
{
  getDesignDocument()->updateElement(id, contentJsonString);
}

std::string VggSdk::getFramesInfo() const
{
  return getModel()->getFramesInfo();
}

int VggSdk::currentFrameIndex() const
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->currentFrame();
    }
  }

  return -1;
}

bool VggSdk::setCurrentFrame(const std::string& name)
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->setCurrentFrame(name);
    }
  }
  return false;
}

int VggSdk::launchFrameIndex() const
{
  return getModel()->getLaunchFrameIndex();
}

bool VggSdk::setLaunchFrame(const std::string& name)
{
  return getModel()->setLaunchFrame(name);
}

std::string VggSdk::requiredFonts() const
{
  if (auto currentEnv = env())
  {
    if (auto presenter = currentEnv->presenter())
    {
      nlohmann::json fonts = presenter->requiredFonts();
      return fonts.dump();
    }
  }

  return {};
}

bool VggSdk::addFont(const uint8_t* data, size_t size, const char* defaultName)
{
  auto result = layer::FontManager::GetFontMananger().addFontFromMemory(data, size, defaultName);

  if (auto currentEnv = env())
  {
    if (auto presenter = currentEnv->presenter())
    {
      presenter->setNeedsReload();
    }
  }

  return result;
}

// event listener
void VggSdk::addEventListener(
  const std::string& elementKey,
  const std::string& eventType,
  const std::string& listenerCode)
{
  getModel()->addEventListener(elementKey, eventType, listenerCode);
}

void VggSdk::removeEventListener(
  const std::string& elementKey,
  const std::string& eventType,
  const std::string& listenerCode)
{
  getModel()->removeEventListener(elementKey, eventType, listenerCode);
}

VggSdk::ListenersType VggSdk::getEventListeners(const std::string& elementKey)
{
#ifdef EMSCRIPTEN
  using namespace emscripten;

  auto result_listeners_map = val::object();
  auto listenersMap = getModel()->getEventListeners(elementKey);
  for (auto& map_item : listenersMap)
  {
    if (map_item.second.empty())
    {
      continue;
    }

    auto& eventType = map_item.first;

    auto js_listener_code_array = val::array();
    for (std::size_t i = 0; i < map_item.second.size(); ++i)
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
  return getModel()->getEventListeners(elementKey);
#endif
}

// vgg model
std::shared_ptr<JsonDocument> VggSdk::getDesignDocument(IndexType index) const
{
  return getModel(index)->runtimeDesignDoc();
}
std::shared_ptr<Daruma> VggSdk::getModel(IndexType index) const
{
  auto key = index == main_or_editor_daruma_index ? DarumaContainer::KeyType::MainOrEditor
                                                  : DarumaContainer::KeyType::Edited;
  if (auto theEnv = env())
  {
    return theEnv->darumaContainer().get(key);
  }
  else
  {
    return nullptr;
  }
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

std::vector<uint8_t> VggSdk::vggFileBuffer()
{
  auto model = getModel();
  if (model)
  {
    auto      saver{ std::make_shared<Model::ZipStreamSaver>() };
    SaveModel saveModel{ model, saver };

    saveModel.save();
    return saver->buffer();
  }
  else
  {
    WARN("VggSdk: no daruma file to save");
    return {};
  }
}

#ifdef EMSCRIPTEN
emscripten::val VggSdk::vggFileUint8Array()
{
  const auto&     buffer = vggFileBuffer();
  emscripten::val array = emscripten::val::global("Uint8Array")
                            .new_(emscripten::typed_memory_view(buffer.size(), buffer.data()));
  return array;
}
#endif