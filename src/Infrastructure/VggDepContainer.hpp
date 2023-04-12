#pragma once

template<typename T>
class VggDepContainer
{
public:
  static T& get()
  {
    return m_obj;
  }
  static void set(const T& obj)
  {
    m_obj = obj;
  }

private:
  VggDepContainer() = default;

  static T m_obj;
};

template<typename T>
T VggDepContainer<T>::m_obj;