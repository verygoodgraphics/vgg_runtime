#include "DarumaContainer.hpp"

using namespace VGG;

void DarumaContainer::add(std::shared_ptr<Daruma> daruma, KeyType key)
{
  getRepo().insert_or_assign(key, daruma);
}

void DarumaContainer::remove(KeyType key)
{
  getRepo().erase(key);
}

std::shared_ptr<Daruma> DarumaContainer::get(KeyType key)
{
  return getRepo()[key];
}
