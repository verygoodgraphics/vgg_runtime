#pragma once

template<typename T>
class VggDepContainer
{
public:
  static T& get()
  {
    static T obj;
    return obj;
  }

private:
  VggDepContainer() = default;
};
