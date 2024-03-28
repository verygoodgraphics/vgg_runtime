/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "DesignDocAdapter.hpp"

#include "Domain/Model/Element.hpp"
#include "Utility/Log.hpp"

using namespace VGG;
using namespace VGG::Domain;
using namespace VGG::Model;

DesignDocAdapter::DesignDocAdapter(std::shared_ptr<VGG::Domain::DesignDocument> designDocTree)
  : m_designDocTree{ designDocTree }
{
  ASSERT(m_designDocTree != nullptr);
}

json DesignDocAdapter::content() const
{
  return m_designDocTree->treeModel();
}

std::string DesignDocAdapter::getElement(const std::string& id)
{
  if (auto element = m_designDocTree->getElementByKey(id))
  {
    auto j = element->jsonModel();
    return j.dump();
  }

  return {};
}

void DesignDocAdapter::updateElement(const std::string& id, const std::string& contentJsonString)
{
  if (auto element = m_designDocTree->getElementByKey(id))
  {
    auto        j = element->jsonModel();
    const auto& patch = nlohmann::json ::parse(contentJsonString);
    j.merge_patch(patch);
    element->updateJsonModel(j);
  }
}