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
#include "StartRunning.hpp"

#include "Domain/Layout/ExpandSymbol.hpp"

using namespace VGG;
using namespace VGG::Layout;

StartRunning::StartRunning(std::shared_ptr<Daruma> model)
{
  ASSERT(model);

  if (model->layoutDoc())
  {
    ExpandSymbol expandSymbol{ model->designDoc()->content(), model->layoutDoc()->content() };

    auto result = expandSymbol();
    m_layout = expandSymbol.layout();

    model->setRuntimeDesignDocTree(std::get<0>(result));
    model->setRuntimeLayoutDoc(std::get<1>(result));
  }
  else
  {
    ExpandSymbol expandSymbol{ model->designDoc()->content() };

    auto result = expandSymbol();
    m_layout = expandSymbol.layout();

    model->setRuntimeDesignDocTree(std::get<0>(result));
  }
}
