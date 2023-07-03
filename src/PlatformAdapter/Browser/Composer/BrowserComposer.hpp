#pragma once

#include "Main/PlatformComposer.hpp"

#include "PlatformAdapter/Browser/Exec/BrowserJSEngine.hpp"

#include <memory>

class BrowserComposer : public PlatformComposer
{
public:
  virtual std::shared_ptr<VggJSEngine> createJsEngine() override
  {
    return std::make_shared<BrowserJSEngine>();
  }
};
