#pragma once

#include <string>

class VggEnv
{
public:
  virtual ~VggEnv() = default;

  virtual const std::string getEnv() = 0;
};
