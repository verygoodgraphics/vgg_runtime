/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WebContainer.hpp"
#include "WebGraphicsContext.hpp"

#include "Common/Container.hpp"

namespace VGG
{

// impl ----------------------------------------------------------------------
class WebContainerImpl : public IContainer
{
  friend WebContainer;

  WebContainer*              m_api;
  std::unique_ptr<Container> m_impl;

public:
  WebContainerImpl(WebContainer* api)
    : m_api(api)
  {
    m_impl.reset(new Container);
  }

  IContainer* container() override
  {
    return m_impl.get();
  }

  void init(int w, int h, float devicePixelRatio)
  {
    auto theContext = new WebGraphicsContext();
    theContext->init(devicePixelRatio);

    std::unique_ptr<VGG::layer::SkiaGraphicsContext> graphicsContext{ theContext };
    m_impl->setGraphicsContext(graphicsContext, w, h);
  }
};

// api ----------------------------------------------------------------------

WebContainer::WebContainer(int w, int h, float devicePixelRatio)
  : m_impl(new WebContainerImpl(this))
{
  m_impl->init(w, h, devicePixelRatio);
}

WebContainer::~WebContainer() = default;

bool WebContainer::jsLoad(const emscripten::val& int8ArrayValue)
{
  unsigned int      length = int8ArrayValue["length"].as<unsigned int>();
  std::vector<char> charVector;
  charVector.resize(length);
  auto memory = emscripten::val::module_property("HEAPU8")["buffer"];
  auto memoryView = int8ArrayValue["constructor"].new_(
    memory,
    reinterpret_cast<uintptr_t>(charVector.data()),
    length);
  memoryView.call<void>("set", int8ArrayValue);

  return m_impl->load(charVector);
}

void WebContainer::jsRun()
{
  run();
}

IContainer* WebContainer::container()
{
  return m_impl.get();
}

} // namespace VGG