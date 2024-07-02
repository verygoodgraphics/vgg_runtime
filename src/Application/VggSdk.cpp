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
#include "VggSdk.hpp"
#include <unordered_set>
#include "Application/ElementAddPropertySerializer.hpp"
#include "Application/ElementDeletePropertySerializer.hpp"
#include "Application/ElementGetPropertySerializer.hpp"
#include "Application/ElementUpdatePropertySerializer.hpp"
#include "Application/Presenter.hpp"
#include "Application/UIApplication.hpp"
#include "Application/VggEnv.hpp"
#include "Controller.hpp"
#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Domain/JsonDocument.hpp"
#include "Domain/Saver/DirSaver.hpp"
#include "Domain/Saver/ZipStreamSaver.hpp"
#include "Layer/FontManager.hpp"
#include "Layer/VGGLayer.hpp"
#include "UIAnimation.hpp"
#include "UIOptions.hpp"
#include "UseCase/SaveModel.hpp"
#include "Utility/Log.hpp"
#include <nlohmann/json.hpp>

#ifdef EMSCRIPTEN
constexpr auto listener_code_key = "listener";
#endif

namespace VGG
{

namespace
{

app::UIAnimationOption toUIAnimationOption(const ISdk::AnimationOptions& inOption)
{
  app::UIAnimationOption option;
  if (inOption.duration > 0)
    option.duration = inOption.duration;

  if (inOption.type == "none")
    option.type = app::EAnimationType::NONE;
  if (inOption.type == "default")
    option.type = app::EAnimationType::DEFAULT;
  else if (inOption.type == "dissolve")
    option.type = app::EAnimationType::DISSOLVE;
  else if (inOption.type == "smart")
    option.type = app::EAnimationType::SMART;

  if (inOption.timingFunction == "linear")
    option.timingFunction = app::EAnimationTimingFunction::LINEAR;

  return option;
}

} // namespace

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
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->getFramesInfo();
    }
  }
  return {};
}

std::string VggSdk::currentFrameId() const
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->currentFrameId();
    }
  }

  return {};
}

bool VggSdk::setCurrentFrameById(const std::string& id, bool resetScrollPosition)
{
  return pushFrame(id, { resetScrollPosition, {} });
}

const std::string VggSdk::launchFrameId() const
{
  return getModel()->launchFrameId();
}
bool VggSdk::setLaunchFrameById(const std::string& id)
{
  return getModel()->setLaunchFrameById(id);
}

const std::string VggSdk::currentTheme() const
{
  return getModel()->currentTheme();
}
bool VggSdk::setCurrentTheme(const std::string& theme)
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->setCurrentTheme(theme);
    }
  }
  return false;
}

bool VggSdk::presentFrameById(const std::string& id, bool resetScrollPosition)
{
  return presentFrame(id, { resetScrollPosition, {} });
}

bool VggSdk::dismissFrame()
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->dismissFrame();
    }
  }
  return false;
}

bool VggSdk::goBack(bool resetScrollPosition, bool resetState)
{
  return popFrame({ resetScrollPosition, resetState });
}

bool VggSdk::nextFrame()
{
  if (auto c = controller())
  {
    return c->nextFrame();
  }

  return false;
}

bool VggSdk::previousFrame()
{
  if (auto c = controller())
  {
    return c->previouseFrame();
  }

  return false;
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
  auto result = layer::FontManager::getFontMananger().addFontFromMemory(data, size, defaultName);

  if (auto currentEnv = env())
  {
    if (auto presenter = currentEnv->presenter())
    {
      presenter->setDirtry();
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

std::vector<std::string> VggSdk::texts()
{
  auto model = getModel();
  if (model)
  {
    auto                     ret = model->texts();
    std::vector<std::string> results{ ret.begin(), ret.end() };
    return results;
  }
  else
  {
    WARN("VggSdk: no daruma file");
    return {};
  }
}

#ifdef EMSCRIPTEN
bool VggSdk::jsAddFont(const emscripten::val& jsFontUint8Array, const std::string& defaultName)
{
  const auto& font = emscripten::convertJSArrayToNumberVector<uint8_t>(jsFontUint8Array);
  return addFont(font.data(), font.size(), defaultName.c_str());
}

emscripten::val VggSdk::vggFileUint8Array()
{
  const auto&     buffer = vggFileBuffer();
  emscripten::val array = emscripten::val::global("Uint8Array")
                            .new_(emscripten::typed_memory_view(buffer.size(), buffer.data()));
  return array;
}
#endif

std::vector<uint8_t> VggSdk::makeImageSnapshot(const ImageOptions& sdkOptions)
{
  layer::ImageOptions options;

  auto& type = sdkOptions.type;
  if (type == "png")
  {
    options.encode = layer::EImageEncode::IE_PNG;
  }
  else if (type == "jpg")
  {
    options.encode = layer::EImageEncode::IE_JPEG;
  }
  else if (type == "webp")
  {
    options.encode = layer::EImageEncode::IE_WEBP;
  }
  else if (type == "raw")
  {
    options.encode = layer::EImageEncode::IE_RAW;
  }

  options.position[0] = 0;
  options.position[1] = 0;
  options.quality = sdkOptions.quality;

  return env()->application()->makeImageSnapshot(options);
}

#ifdef EMSCRIPTEN
emscripten::val VggSdk::emMakeImageSnapshot(const ImageOptions& options)
{
  const auto&     buffer = makeImageSnapshot(options);
  emscripten::val array = emscripten::val::global("Uint8Array")
                            .new_(emscripten::typed_memory_view(buffer.size(), buffer.data()));
  return array;
}
#endif

bool VggSdk::setState(
  const std::string&  instanceDescendantId,
  const std::string&  listenerId,
  const std::string&  masterId,
  const StateOptions& options)
{
  if (auto c = controller())
  {
    app::StateOptions opts{ options.resetScrollPosition, toUIAnimationOption(options.animation) };
    return c->setState(instanceDescendantId, listenerId, masterId, opts);
  }

  return false;
}

bool VggSdk::presentState(
  const std::string&  instanceDescendantId,
  const std::string&  listenerId,
  const std::string&  stateMasterId,
  const StateOptions& options)
{
  if (auto c = controller())
  {
    app::StateOptions opts{ options.resetScrollPosition, toUIAnimationOption(options.animation) };
    return c->presentState(instanceDescendantId, listenerId, stateMasterId, opts);
  }
  return false;
}

bool VggSdk::dismissState(const std::string& instanceDescendantId)
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      return controller->dismissState(instanceDescendantId);
    }
  }
  return false;
}

void VggSdk::setFitToViewportEnabled(bool enabled)
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      controller->setFitToViewportEnabled(enabled);
    }
  }
}

void VggSdk::openUrl(const std::string& url, const std::string& target)
{
  if (auto currentEnv = env())
  {
    if (auto controller = currentEnv->controller())
    {
      controller->openUrl(url, target);
    }
  }
}

std::shared_ptr<VGG::Controller> VggSdk::controller()
{
  if (const auto& currentEnv = env())
  {
    return currentEnv->controller();
  }

  return {};
}

void VggSdk::setContentMode(const std::string& mode)
{
  if (auto c = controller())
  {
    c->setContentMode(mode);
  }
}

bool VggSdk::setCurrentFrameByIdAnimated(
  const std::string&      id,
  bool                    resetScrollPosition,
  const AnimationOptions& inOption)
{
  return pushFrame(id, { resetScrollPosition, inOption });
}

bool VggSdk::pushFrame(const std::string& id, const FrameOptions& inOpts)
{
  auto c = controller();
  if (!c)
    return false;

  app::FrameOptions opts{ inOpts.resetScrollPosition, toUIAnimationOption(inOpts.animation) };
  return c->pushFrame(id, opts);
}
bool VggSdk::popFrame(const app::PopOptions& opts)
{
  auto c = controller();
  if (!c)
    return false;

  return c->popFrame(opts);
}
bool VggSdk::presentFrame(const std::string& id, const FrameOptions& inOpts)
{
  auto c = controller();
  if (!c)
    return false;

  app::FrameOptions opts{ inOpts.resetScrollPosition, toUIAnimationOption(inOpts.animation) };
  return c->presentFrame(id, opts);
}

void VggSdk::setBackgroundColor(uint32_t color)
{
  if (const auto& p = presenter())
    p->setBackgroundColor(color);
}

std::shared_ptr<Presenter> VggSdk::presenter()
{
  if (const auto& currentEnv = env())
    return currentEnv->presenter();

  return nullptr;
}

int VggSdk::updateElementProperties(const std::string& updates, const AnimationOptions& animation)
{
  if (const auto& p = presenter())
  {
    const auto& j = nlohmann::json::parse(updates);
    return p->updateElement(j, toUIAnimationOption(animation));
  }

  return 0;
}

std::string VggSdk::getElementProperty(const std::string& query)
{
  if (const auto& p = presenter())
  {
    const auto& j = nlohmann::json::parse(query);
    const auto& r = p->getElementProperty(j);

    nlohmann::json result = r;
    return result.dump();
  }

  return {};
}

bool VggSdk::addElementProperty(const std::string& command)
{
  if (const auto& p = presenter())
  {
    auto j = nlohmann::json::parse(command);
    return p->addElementProperty(j);
  }
  return false;
}

bool VggSdk::deleteElementProperty(const std::string& command)
{
  if (const auto& p = presenter())
  {
    auto j = nlohmann::json::parse(command);
    return p->deleteElementProperty(j);
  }
  return false;
}

} // namespace VGG
