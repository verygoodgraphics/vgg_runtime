#include "Controller.hpp"

#include "RawJsonDocument.hpp"
#include "VggDepContainer.hpp"
#include "VggExec.hpp"
#include "VggWork.hpp"

Controller::Controller(RunMode mode)
  : m_mode(mode)
{
}

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

void Controller::onClick(const std::string& path)
{
  auto code = m_work->getCode(path);
  vggExec()->evalModule(code);
}

const std::shared_ptr<VggExec>& Controller::vggExec()
{
  return VggDepContainer<std::shared_ptr<VggExec>>::get();
}