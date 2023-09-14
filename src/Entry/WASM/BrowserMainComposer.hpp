#pragma once

#include "Application/MainComposer.hpp"
#include "Adapter/BrowserComposer.hpp"

#include <Application/Mouse.hpp>

class VggBrowser
{
public:
  VggBrowser() = delete;

  static VGG::MainComposer& mainComposer()
  {
    static VGG::MainComposer instance{ new BrowserComposer(), std::make_shared<Mouse>() };
    return instance;
  }
};