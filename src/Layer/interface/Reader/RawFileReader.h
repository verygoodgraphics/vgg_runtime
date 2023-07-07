#pragma once

#include "Common/Config.h"
#include "IReader.hpp"

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
    const auto cmd = genCmd(fullpath);
    auto ret = executeExternalCmd(cmd);
    Data data;
    if (ret == 0)
    {
      auto readFile = getReadFile();
      data.Format = readJson(getReadFile());
      data.Resource = readRes(readFile.stem() / config.at("outputImageDir"));
    }
    return data;
  }
};
} // namespace VGG
