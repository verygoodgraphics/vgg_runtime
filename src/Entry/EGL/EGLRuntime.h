#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <tuple>
#include <map>
#include <nlohmann/json.hpp>
#include "include/core/SkData.h"

int init(int width, int height);

std::tuple<std::string, std::map<int, sk_sp<SkData>>> render(
  const nlohmann::json& j,
  const std::map<std::string, std::vector<char>>& resources,
  int imageQuality);
