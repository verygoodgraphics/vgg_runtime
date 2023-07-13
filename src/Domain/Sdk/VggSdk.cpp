#include "VggSdk.hpp"

#include "Utils/DIContainer.hpp"

#ifdef EMSCRIPTEN
constexpr auto listener_code_key = "listener";
#endif

// design document in vgg work
const std::string VggSdk::designDocument()
{
  return getDesignDocument()->content().dump();
}

void VggSdk::designDocumentReplaceAt(const std::string& json_pointer, const std::string& value)
{
  getDesignDocument()->replaceAt(json_pointer, value);
}

void VggSdk::designDocumentAddAt(const std::string& json_pointer, const std::string& value)
{
  getDesignDocument()->addAt(json_pointer, value);
}

void VggSdk::designDocumentDeleteAt(const std::string& json_pointer)
{
  getDesignDocument()->deleteAt(json_pointer);
}

// event listener
void VggSdk::addEventListener(const std::string& element_path,
                              const std::string& event_type,
                              const std::string& listener_code)
{
  getVggWork()->addEventListener(element_path, event_type, listener_code);
}

void VggSdk::removeEventListener(const std::string& element_path,
                                 const std::string& event_type,
                                 const std::string& listener_code)
{
  getVggWork()->removeEventListener(element_path, event_type, listener_code);
}

VggSdk::ListenersType VggSdk::getEventListeners(const std::string& element_path)
{
#ifdef EMSCRIPTEN
  using namespace emscripten;

  auto result_listeners_map = val::object();
  auto listeners_map = getVggWork()->getEventListeners(element_path);
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
  return getVggWork()->getEventListeners(element_path);
#endif
}

// vgg work
std::shared_ptr<JsonDocument> VggSdk::getDesignDocument()
{
  return getVggWork()->designDoc();
}

std::shared_ptr<VggWork> VggSdk::getVggWork()
{
  return VGG::DIContainer<std::shared_ptr<VggWork>>::get();
}