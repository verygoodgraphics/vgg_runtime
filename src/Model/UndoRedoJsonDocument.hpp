#pragma once

#include "JsonDocument.hpp"

#include "Automerge.h"

#include <memory>

class UndoRedoJsonDocument : public JsonDocument
{
public:
  virtual void setContent(const nlohmann::json& content) override
  {
    from_json(content, m_doc);
  }
  virtual const json& content() const override
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
