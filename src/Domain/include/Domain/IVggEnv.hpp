#pragma once

#include <string>

class IVggEnv
{
public:
  virtual ~IVggEnv() = default;

  virtual const std::string getEnv() = 0;
};
