#include "Sdk/VggSdk.hpp"

#include "Utils/DIContainer.hpp"

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

VggWork::ListenersType VggSdk::getEventListeners(const std::string& element_path)
{
  return getVggWork()->getEventListeners(element_path);
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