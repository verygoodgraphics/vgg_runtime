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
#pragma once
#include "Domain/JsonDocument.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Domain/Layout/Layout.hpp"
#include "Domain/RawJsonDocument.hpp"

#include <memory>
#include <nlohmann/json.hpp>
#include <Utility/Log.hpp>
#include <optional>

#define SET_BUILDER_OPTION(container, attr)                                                        \
  ASSERT(!this->m_invalid);                                                                        \
  this->container = std::move(attr);                                                               \
  return std::move(*this);

namespace VGG::entry
{
using namespace nlohmann;

class DocBuilder
{
  bool m_enableExpand{ true };
  bool m_enableLayout{ true };
  json m_layout;
  json m_doc;
  bool m_invalid{ false };

  void moveToThis(DocBuilder&& that) noexcept
  {
    ASSERT(!that.m_invalid);
    m_enableExpand = that.m_enableExpand;
    m_enableLayout = that.m_enableLayout;
    m_layout = std::move(that.m_layout);
    m_doc = std::move(that.m_doc);
    that.m_invalid = true;
  }

  DocBuilder() = default;

public:
  struct Result
  {
    std::string         info;
    std::optional<json> doc;
    Result(std::string info, json doc)
      : info(std::move(info))
      , doc(std::move(doc))
    {
    }
  };

  DocBuilder(const DocBuilder&) = delete;
  DocBuilder& operator=(const DocBuilder&) = delete;

  DocBuilder(DocBuilder&& other) noexcept
  {
    moveToThis(std::move(other));
  }

  DocBuilder& operator=(DocBuilder&& other) noexcept
  {
    moveToThis(std::move(other));
    return *this;
  }

  DocBuilder setLayout(json layout)
  {
    SET_BUILDER_OPTION(m_layout, layout);
  }
  DocBuilder setDocument(json doc)
  {
    SET_BUILDER_OPTION(m_doc, doc);
  }
  DocBuilder setExpandEnabled(bool enabled)
  {
    SET_BUILDER_OPTION(m_enableExpand, enabled);
  }
  DocBuilder setLayoutEnabled(bool enabled)
  {
    SET_BUILDER_OPTION(m_enableLayout, enabled);
  }

  Result build()
  {
    ASSERT(!m_invalid);
    std::string info;
    if (m_enableExpand)
    {
      m_doc = Layout::ExpandSymbol(m_doc, m_layout)();
      if (m_enableLayout)
      {

        JsonDocumentPtr docPtr = std::make_shared<RawJsonDocument>();
        docPtr->setContent(m_doc);
        JsonDocumentPtr layoutPtr = std::make_shared<RawJsonDocument>();
        layoutPtr->setContent(m_layout);
        Layout::Layout layout(std::move(docPtr), std::move(layoutPtr), true);
        m_doc = layout.displayDesignDoc()->content();
      }
    }
    Result res{ std::move(info), std::move(m_doc) };
    m_invalid = true;
    return res;
  }

  static DocBuilder builder()
  {
    return DocBuilder();
  }
};
} // namespace VGG::entry
