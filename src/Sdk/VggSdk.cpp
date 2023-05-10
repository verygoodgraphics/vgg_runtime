#include "Sdk/VggSdk.hpp"

#include "Model/VggWork.hpp"
#include "Utils/DIContainer.hpp"

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

std::shared_ptr<JsonDocument>& VggSdk::getDesignDocument()
{
  auto vggWork = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  return vggWork->designDoc();
}
