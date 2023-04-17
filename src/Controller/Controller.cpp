#include "Controller.hpp"

#include "RawJsonDocument.hpp"
#include "VggDepContainer.hpp"
#include "VggWork.hpp"

bool Controller::start(const std::string& filePath)
{
  auto fn = [](const json& design_json)
  {
    auto raw_json_doc = new RawJsonDocument();
    raw_json_doc->setContent(design_json);
    return JsonDocumentPtr(raw_json_doc);
  };
  m_work.reset(new VggWork(fn));
  VggDepContainer<std::shared_ptr<VggWork>>::get() = m_work;

  return m_work->load(filePath);
}