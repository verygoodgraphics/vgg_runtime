#pragma once

template<typename T>
class VggDepContainer
{
public:
  static T& get()
  {
    return m_obj;
  }

private:
  VggDepContainer() = default;

  static T m_obj;
};

template<typename T>
T VggDepContainer<T>::m_obj;