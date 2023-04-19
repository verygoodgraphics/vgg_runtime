#include "SubjectJsonDocument.hpp"

#include <algorithm>

void SubjectJsonDocument::addObserver(const JsonDocumentObserverPtr& observer)
{
  m_observers.push_back(observer);
}

void SubjectJsonDocument::removeObserver(const JsonDocumentObserverPtr& observer)
{
  m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer),
                    m_observers.end());
}

void SubjectJsonDocument::addAt(const json::json_pointer& path, const json& value)
{
  JsonDocument::addAt(path, value);

  for (auto& observer : m_observers)
  {
    observer->didAdd(path, value);
  }
}

void SubjectJsonDocument::replaceAt(const json::json_pointer& path, const json& value)
{
  JsonDocument::replaceAt(path, value);

  for (auto& observer : m_observers)
  {
    observer->didUpdate(path, value);
  }
}

void SubjectJsonDocument::deleteAt(const json::json_pointer& path)
{
  JsonDocument::deleteAt(path);

  for (auto& observer : m_observers)
  {
    observer->didDelete(path);
  }
}