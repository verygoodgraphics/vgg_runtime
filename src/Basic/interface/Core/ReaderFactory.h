#pragma once

#include "IReader.hpp"
#include "RawFileReader.h"
#include "SketchFileReader.h"

namespace VGG
{

// A naive factory pattern

inline std::shared_ptr<IReader> GetSketchReader(const std::string& filename)
{
  return std::make_shared<SketchFileReader>(filename);
}

inline std::shared_ptr<IReader> GetRawReader(const std::string& filename, const std::string& resDir)
{
  return std::make_shared<RawFileReader>(filename, resDir);
}

}; // namespace VGG
