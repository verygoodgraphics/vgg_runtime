#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <tuple>
#include <map>
#include <nlohmann/json.hpp>
#include "include/core/SkData.h"

/**
 * @brief 
 * 
 * @param j vgg_format json object 
 * @param resources blob resources map
 * @param imageQuality 0 - 100, 100 is the best quality of the generated image
 * @return std::tuple<std::string, std::map<int, sk_sp<SkData>>> 
 * 
 * first element is reason string, error ocurrs if string is not empty.
 * the second element is the render result, the key is the index of the artboard,
 * the value is the binary image data, which format is PNG.
 */
std::tuple<std::string, std::map<int, sk_sp<SkData>>> render(
  const nlohmann::json& j,
  const std::map<std::string, std::vector<char>>& resources,
  int imageQuality);
