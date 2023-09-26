#pragma once

#include "Entry/SDL/SdlMouse.hpp"

#include "Adapter/BrowserComposer.hpp"
#include "Application/MainComposer.hpp"

class VggBrowser
{
public:
  VggBrowser() = delete;

  static VGG::MainComposer& mainComposer()
  {
    static VGG::MainComposer s_instance{ new BrowserComposer(), std::make_shared<SdlMouse>() };
    return s_instance;
  }
};