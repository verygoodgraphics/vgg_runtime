#pragma once

#include "Exec/IVggEnv.hpp"

#include <sstream>
#include <string>

class VggEnv : public IVggEnv
{
public:
  const std::string getEnv()
  {
    const void* address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << address;
    return ss.str();
  }
};
