#pragma once

#include "JsonDocument.hpp"

#include "ModelEvent.hpp"

#include <rxcpp/rx.hpp>

#include <memory>
#include <vector>

class SubjectJsonDocument : public JsonDocument
{
  rxcpp::subjects::subject<VGG::ModelEventPtr> m_subject;

public:
  SubjectJsonDocument(JsonDocumentPtr jsonDoc)
    : JsonDocument(jsonDoc)
  {
  }

  auto getObservable()
  {
    return m_subject.get_observable();
  }

  virtual void addAt(const json::json_pointer& path, const json& value) override;
  virtual void replaceAt(const json::json_pointer& path, const json& value) override;
  virtual void deleteAt(const json::json_pointer& path) override;
};
