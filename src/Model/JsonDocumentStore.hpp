#pragma once

#include "DocumentStore.hpp"

using json = nlohmann::json;

class JsonDocumentStore : public DocumentStore
{
public:
  virtual void setDocument(const nlohmann::json& document) override
  {
    m_doc = document;
  }
  virtual const json& document() const override
  {
    return m_doc;
  }

  virtual void addAt(const json::json_pointer& path, const json& value) override
  {
    m_doc[path] = value;
  }

  virtual void replaceAt(const json::json_pointer& path, const json& value) override
  {
    m_doc[path] = value;
  }

  virtual void deleteAt(const json::json_pointer& path) override
  {
    m_doc.at(path.parent_pointer()).erase(path.back());
  }

private:
  json m_doc;
};
