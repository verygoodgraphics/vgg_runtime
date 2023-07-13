#pragma once

#include "VggEnv.hpp"
#include "Domain/VggExec.hpp"
#include "Domain/VggJSEngine.hpp"
#include "Utils/DIContainer.hpp"

class PlatformComposer
{
public:
  virtual ~PlatformComposer() = default;

  void setup()
  {
    auto exec = std::make_shared<VggExec>(createJsEngine(), std::make_shared<VggEnv>());
    VGG::DIContainer<std::shared_ptr<VggExec>>::get() = exec;

    platformSetup();
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
