#pragma once

#include "Daruma.hpp"

#include <unordered_map>
#include <memory>

namespace VGG
{
class DarumaContainer
{
public:
  enum class KeyType
  {
    MainOrEditor,
    Edited
  };

  void add(std::shared_ptr<Daruma> daruma, KeyType key = KeyType::MainOrEditor);
  void remove(KeyType key = KeyType::MainOrEditor);
  std::shared_ptr<Daruma>& get(KeyType key = KeyType::MainOrEditor);

private:
  auto& getRepo()
  {
    static std::unordered_map<KeyType, std::shared_ptr<Daruma>> repo;
    return repo;
  }
};

} // namespace VGG