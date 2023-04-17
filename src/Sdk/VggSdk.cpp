#include "VggSdk.hpp"

#include "VggDepContainer.hpp"
#include "VggWork.hpp"

void VggSdk::updateStyle()
{
}

// const std::string VggSdk::findElement(const std::string& content)
// {
//   return designDocument()->findElement(content);
// }

const std::string VggSdk::documentJson()
{
  return designDocument()->content().dump();
}

// const std::string VggSdk::getElementPath(const std::string& content)
// {
//   return designDocument()->getElementPath(content);
// }

// const std::string VggSdk::getElementContainerPath(const std::string& content)
// {
//   return designDocument()->getElementContainerPath(content);
// }

const std::string VggSdk::jsonAt(const std::string& json_pointer)
{
  return designDocument()->content()[json_pointer];
}

void VggSdk::replaceInDocument(const std::string& json_pointer, const std::string& value)
{
  designDocument()->replaceAt(json::json_pointer(json_pointer), value);
}

void VggSdk::addToDocument(const std::string& json_pointer, const std::string& value)
{
}

void VggSdk::deleteFromDocument(const std::string& json_pointer)
{
}

std::shared_ptr<JsonDocument>& VggSdk::designDocument()
{
  auto vggWork = VggDepContainer<std::shared_ptr<VggWork>>::get();
  return vggWork->designDoc();
}