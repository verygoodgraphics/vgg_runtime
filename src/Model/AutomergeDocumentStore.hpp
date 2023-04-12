#pragma once

#include "Automerge.h"
#include "DocumentStore.hpp"

using json = nlohmann::json;

class AutomergeDocumentStore : public DocumentStore
{
public:
  virtual void setDocument(const nlohmann::json& document) override
  {
    from_json(document, m_doc);
  }
  virtual const json& document() const override
  {
    return m_doc.json_const_ref();
  }

  virtual void addAt(const json::json_pointer& path, const json& value) override
  {
    m_doc.json_add(path, value);
  }

  virtual void replaceAt(const json::json_pointer& path, const json& value) override
  {
    m_doc.json_replace(path, value);
  }

  virtual void deleteAt(const json::json_pointer& path) override
  {
    m_doc.json_delete(path);
  }

private:
  Automerge m_doc;
};
