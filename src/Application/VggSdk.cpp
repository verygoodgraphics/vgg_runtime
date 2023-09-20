#include "VggSdk.hpp"

#include "Utility/DIContainer.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Domain/Saver/DirSaver.hpp"
#include "Utility/Log.h"
#include "UseCase/SaveModel.hpp"

#ifdef EMSCRIPTEN
constexpr auto listener_code_key = "listener";
#endif

using namespace VGG;

// design document in vgg daruma file
const std::string VggSdk::designDocument(IndexType index)
{
  return getDesignDocument(index)->content().dump();
}

void VggSdk::designDocumentReplaceAt(const std::string& jsonPointer,
                                     const std::string& value,
                                     IndexType index)
{
  getDesignDocument(index)->replaceAt(jsonPointer, value);
}

void VggSdk::designDocumentAddAt(const std::string& jsonPointer,
                                 const std::string& value,
                                 IndexType index)
{
  getDesignDocument(index)->addAt(jsonPointer, value);
}

void VggSdk::designDocumentDeleteAt(const std::string& jsonPointer, IndexType index)
{
  getDesignDocument(index)->deleteAt(jsonPointer);
}

// event listener
void VggSdk::addEventListener(const std::string& elementPath,
                              const std::string& eventType,
                              const std::string& listenerCode,
                              IndexType index)
{
  getModel(index)->addEventListener(elementPath, eventType, listenerCode);
}

void VggSdk::removeEventListener(const std::string& elementPath,
                                 const std::string& eventType,
                                 const std::string& listenerCode,
                                 IndexType index)
{
  getModel(index)->removeEventListener(elementPath, eventType, listenerCode);
}

VggSdk::ListenersType VggSdk::getEventListeners(const std::string& elementPath, IndexType index)
{
#ifdef EMSCRIPTEN
  using namespace emscripten;

  auto result_listeners_map = val::object();
  auto listenersMap = getModel(index)->getEventListeners(elementPath);
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
      auto js_listener_object = val::object();

      js_listener_object.set(listener_code_key, val(listenerCode.c_str()));
      js_listener_code_array.set(i, js_listener_object);
    }

    result_listeners_map.set(eventType.c_str(), js_listener_code_array);
  }

  return result_listeners_map;
#else
  return getModel(index)->getEventListeners(elementPath);
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
    auto saver{ std::make_shared<Model::DirSaver>(dstDir) };
    Editor editor{ editModel, saver };

    editor.save();
  }
  else
  {
    WARN("VggSdk: no daruma file to save");
  }
}