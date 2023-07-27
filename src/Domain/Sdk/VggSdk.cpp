#include "VggSdk.hpp"

#include "DIContainer.hpp"
#include "Domain/DarumaContainer.hpp"

#ifdef EMSCRIPTEN
constexpr auto listener_code_key = "listener";
#endif

using namespace VGG;

// design document in vgg daruma file
const std::string VggSdk::designDocument(IndexType index)
{
  return getDesignDocument(index)->content().dump();
}

void VggSdk::designDocumentReplaceAt(const std::string& json_pointer,
                                     const std::string& value,
                                     IndexType index)
{
  getDesignDocument(index)->replaceAt(json_pointer, value);
}

void VggSdk::designDocumentAddAt(const std::string& json_pointer,
                                 const std::string& value,
                                 IndexType index)
{
  getDesignDocument(index)->addAt(json_pointer, value);
}

void VggSdk::designDocumentDeleteAt(const std::string& json_pointer, IndexType index)
{
  getDesignDocument(index)->deleteAt(json_pointer);
}

// event listener
void VggSdk::addEventListener(const std::string& element_path,
                              const std::string& event_type,
                              const std::string& listener_code,
                              IndexType index)
{
  getModel(index)->addEventListener(element_path, event_type, listener_code);
}

void VggSdk::removeEventListener(const std::string& element_path,
                                 const std::string& event_type,
                                 const std::string& listener_code,
                                 IndexType index)
{
  getModel(index)->removeEventListener(element_path, event_type, listener_code);
}

VggSdk::ListenersType VggSdk::getEventListeners(const std::string& element_path, IndexType index)
{
#ifdef EMSCRIPTEN
  using namespace emscripten;

  auto result_listeners_map = val::object();
  auto listeners_map = getModel(index)->getEventListeners(element_path);
  for (auto& map_item : listeners_map)
  {
    if (map_item.second.empty())
    {
      continue;
    }

    auto& event_type = map_item.first;

    auto js_listener_code_array = val::array();
    for (int i = 0; i < map_item.second.size(); ++i)
    {
      auto& listener_code = map_item.second[i];
      auto js_listener_object = val::object();

      js_listener_object.set(listener_code_key, val(listener_code.c_str()));
      js_listener_code_array.set(i, js_listener_object);
    }

    result_listeners_map.set(event_type.c_str(), js_listener_code_array);
  }

  return result_listeners_map;
#else
  return getModel(index)->getEventListeners(element_path);
#endif
}

// vgg model
std::shared_ptr<JsonDocument> VggSdk::getDesignDocument(IndexType index)
{
  return getModel(index)->designDoc();
}
std::shared_ptr<Daruma> VggSdk::getModel(IndexType index)
{
  auto key = index == main_or_editor_daruma_index ? DarumaContainer::KeyType::MainOrEditor
                                                  : DarumaContainer::KeyType::Edited;
  return DarumaContainer().get(key);
}
