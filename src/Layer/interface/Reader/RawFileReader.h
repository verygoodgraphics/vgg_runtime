#pragma once

#include "Common/Config.h"
#include "IReader.hpp"
#include "boost/numeric/ublas/fwd.hpp"

#include <iostream>
#include <string>
#include <filesystem>

namespace VGG
{

class VGG_EXPORTS RawFileReader : public IReader
{
public:
  RawFileReader()
  {
  }
  Data read(const fs::path& fullpath) override
  {
    Data data;
    data.Format = readJson(fullpath);
    auto stem = fullpath.parent_path() / "image";
    data.Resource = readRes(stem);
    return data;
  }
};
} // namespace VGG
