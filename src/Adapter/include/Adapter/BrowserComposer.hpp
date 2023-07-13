#pragma once

#include "Application/PlatformComposer.hpp"

#include "BrowserJSEngine.hpp"

#include <memory>

class BrowserComposer : public PlatformComposer
{
public:
  virtual std::shared_ptr<VggJSEngine> createJsEngine() override
  {
    return std::make_shared<BrowserJSEngine>();
  }
};
