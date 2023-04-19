#pragma once

#include "JsonDocument.hpp"

#include <memory>
#include <vector>

class JsonDocumentObserver
{
public:
  virtual ~JsonDocumentObserver() = default;

  virtual void didAdd(const json::json_pointer& path, const json& value) = 0;
  virtual void didUpdate(const json::json_pointer& path, const json& value) = 0;
  virtual void didDelete(const json::json_pointer& path) = 0;
};
using JsonDocumentObserverPtr = std::shared_ptr<JsonDocumentObserver>;

class SubjectJsonDocument : public JsonDocument
{
public:
  SubjectJsonDocument(JsonDocumentPtr jsonDoc)
    : JsonDocument(jsonDoc)
  {
  }

  void addObserver(const JsonDocumentObserverPtr& observer);
  void removeObserver(const JsonDocumentObserverPtr& observer);

  virtual void addAt(const json::json_pointer& path, const json& value) override;
  virtual void replaceAt(const json::json_pointer& path, const json& value) override;
  virtual void deleteAt(const json::json_pointer& path) override;

private:
  std::vector<JsonDocumentObserverPtr> m_observers;
};
