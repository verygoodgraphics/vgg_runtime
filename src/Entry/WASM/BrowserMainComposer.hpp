#pragma once

#include "../SDL/SDLImpl/SdlMouse.hpp"

#include <Adapter/BrowserComposer.hpp>
#include <Application/MainComposer.hpp>

class VggBrowser
{
public:
  VggBrowser() = delete;

  static VGG::MainComposer& mainComposer()
  {
    static VGG::MainComposer instance{ new BrowserComposer(), std::make_shared<SdlMouse>() };
    return instance;
  }
};