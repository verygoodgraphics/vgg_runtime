#include "SubjectJsonDocument.hpp"

#include <algorithm>

using namespace VGG;

void SubjectJsonDocument::addAt(const json::json_pointer& path, const json& value)
{
  JsonDocument::addAt(path, value);

  m_subject.get_subscriber().on_next(ModelEventPtr{ new ModelEventAdd{ path, value } });
}

void SubjectJsonDocument::replaceAt(const json::json_pointer& path, const json& value)
{
  JsonDocument::replaceAt(path, value);

  m_subject.get_subscriber().on_next(ModelEventPtr{ new ModelEventUpdate{ path, value } });
}

void SubjectJsonDocument::deleteAt(const json::json_pointer& path)
{
  JsonDocument::deleteAt(path);

  m_subject.get_subscriber().on_next(ModelEventPtr{ new ModelEventDelete{ path } });
}