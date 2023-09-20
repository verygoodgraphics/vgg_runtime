#pragma once

#include "VggEnv.hpp"
#include "Domain/VggExec.hpp"
#include "Domain/VggJSEngine.hpp"
#include "Utility/DIContainer.hpp"

class PlatformComposer
{
public:
  virtual ~PlatformComposer() = default;

  auto setup()
  {
    auto jsEngine = std::make_shared<VggExec>(createJsEngine(), std::make_shared<VggEnv>());
    VGG::DIContainer<std::shared_ptr<VggExec>>::get() = jsEngine;

    platformSetup();

    return jsEngine;
  }

  void teardown()
  {
    VGG::DIContainer<std::shared_ptr<VggExec>>::get().reset();
    platformTeardown();
  }

  virtual std::shared_ptr<VggJSEngine> createJsEngine() = 0;

  virtual void platformSetup()
  {
  }
  virtual void platformTeardown()
  {
  }
};
