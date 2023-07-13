
#pragma once

#include "JsonDocument.hpp"

#include <memory>

class RawJsonDocument : public JsonDocument
{
public:
  virtual void setContent(const nlohmann::json& content) override
  {
    m_doc = content;
  }
  virtual const json& content() const override
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
    erase(m_doc, path);
  }

private:
  json m_doc;
};
