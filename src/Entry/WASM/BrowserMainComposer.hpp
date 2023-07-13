#pragma once

#include "Application/MainComposer.hpp"
#include "Adapter/BrowserComposer.hpp"

class VggBrowser
{
public:
  VggBrowser() = delete;

  static VGG::MainComposer& mainComposer()
  {
    static VGG::MainComposer instance{ new BrowserComposer() };
    return instance;
  }
};