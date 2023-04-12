#include "VggSdk.hpp"

#include "DocumentModel.hpp"
#include "VggDepContainer.hpp"

void VggSdk::updateStyle()
{
}

// const std::string VggSdk::findElement(const std::string& content)
// {
//   return documentModel()->findElement(content);
// }

const std::string VggSdk::documentJson()
{
  return documentModel()->documentJson().dump();
}

// const std::string VggSdk::getElementPath(const std::string& content)
// {
//   return documentModel()->getElementPath(content);
// }

// const std::string VggSdk::getElementContainerPath(const std::string& content)
// {
//   return documentModel()->getElementContainerPath(content);
// }

const std::string VggSdk::jsonAt(const std::string& json_pointer)
{
  return documentModel()->documentJson()[json_pointer];
}

void VggSdk::replaceInDocument(const std::string& json_pointer, const std::string& value)
{
  documentModel()->replaceAt(json::json_pointer(json_pointer), value);
}

void VggSdk::addToDocument(const std::string& json_pointer, const std::string& value)
{
}

void VggSdk::deleteFromDocument(const std::string& json_pointer)
{
}

std::shared_ptr<DocumentModel>& VggSdk::documentModel()
{
  return VggDepContainer<std::shared_ptr<DocumentModel>>::get();
}