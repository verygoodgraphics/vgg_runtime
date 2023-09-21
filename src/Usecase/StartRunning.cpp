#include "StartRunning.hpp"

#include "Domain/Layout/ExpandSymbol.hpp"

using namespace VGG;
using namespace VGG::Layout;

StartRunning::StartRunning(std::shared_ptr<Daruma> model)
{
  ASSERT(model);

  JsonDocumentPtr designDoc, layoutDoc;
  if (model->layoutDoc())
  {
    auto result =
      ExpandSymbol{ model->designDoc()->content(), model->layoutDoc()->content() }.run();

    model->setRuntimeDesignDoc(std::get<0>(result));
    model->setRuntimeLayoutDoc(std::get<1>(result));
    layoutDoc = model->runtimeLayoutDoc();
  }
  else
  {
    auto designJson = ExpandSymbol{ model->designDoc()->content() }();
    model->setRuntimeDesignDoc(designJson);
  }

  designDoc = model->runtimeDesignDoc();

  m_layout.reset(new Layout::Layout{ designDoc, layoutDoc });
}