#pragma once

namespace VGG
{

template<typename T>
class DIContainer
{
public:
  static T& get()
  {
    static T obj;
    return obj;
  }

private:
  DIContainer() = default;
};

}
