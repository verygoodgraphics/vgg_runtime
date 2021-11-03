/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __TEXTURE_MANAGER_HPP__
#define __TEXTURE_MANAGER_HPP__

#include <optional>
#include <unordered_map>
#include <skia/include/core/SkImage.h>
#include <skia/include/core/SkData.h>
#include <skia/include/core/SkTileMode.h>
#include <skia/src/gpu/gl/GrGLTexture.h>

#include "Utils/FileUtils.hpp"
#include "Utils/Utils.hpp"

namespace VGG
{

struct TextureManager
{
#ifdef EMSCRIPTEN
  inline static std::optional<std::function<void(const std::string&)>> onImageAdded;
#endif
  std::unordered_map<std::string, sk_sp<SkImage>> images;

  inline bool contains(const std::string& name)
  {
    return images.find(name) != images.end();
  }

  std::string addImage(const std::string& name, sk_sp<SkImage> image)
  {
    auto key = name;
    while (contains(key))
    {
      key += "_";
    }
    images[key] = image;
    return key;
  }

protected: // protected methods
  TextureManager()
  {
  }

public: // public static methods
  static TextureManager* getInstance()
  {
    static TextureManager tm;
    return &tm;
  }

  static std::vector<std::string> getImageNames()
  {
    auto tm = getInstance();
    ASSERT(tm);
    std::vector<std::string> names;
    for (auto& kv : tm->images)
    {
      names.push_back(kv.first);
    }
    return names;
  }

  static std::optional<std::string> loadImageFile(const std::string& fp)
  {
    auto tm = getInstance();
    ASSERT(tm);

    if (auto data = SkData::MakeFromFileName(fp.c_str()))
    {
      if (auto image = SkImage::MakeFromEncoded(data))
      {
        auto name = FileUtils::getFileName(fp);
        return tm->addImage(name, image);
      }
      WARN("Failed to decode image: %s", fp.c_str());
    }
    else
    {
      WARN("Failed to load image from file: %s", fp.c_str());
    }
    return std::nullopt;
  }

  static std::optional<std::string> loadImageFromMem(const void* buf,
                                                     size_t len,
                                                     const std::string& name)
  {
    auto tm = getInstance();
    ASSERT(tm);

    if (auto data = SkData::MakeFromMalloc(buf, len))
    {
      if (auto image = SkImage::MakeFromEncoded(data))
      {
        auto key = tm->addImage(FileUtils::getFileName(name), image);
#ifdef EMSCRIPTEN
        if (onImageAdded)
        {
          onImageAdded.value()(key);
          onImageAdded.reset();
        }
#endif
        return key;
      }
      WARN("Failed to decode image: %s", name.c_str());
    }
    else
    {
      WARN("Failed to load image from memory: (0x%p, %lu, %s)", buf, len, name.c_str());
    }
#ifdef EMSCRIPTEN
    if (onImageAdded)
    {
      onImageAdded.reset();
    }
#endif
    return std::nullopt;
  }

  static std::optional<std::string> loadImageBlob(const std::string& fullName,
                                                  const std::string& data)
  {
    auto tm = getInstance();
    ASSERT(tm);

    if (auto skData = SkData::MakeWithCopy(data.data(), data.size()))
    {
      if (auto image = SkImage::MakeFromEncoded(skData))
      {
        auto name = FileUtils::getFileName(fullName);
        return tm->addImage(name, image);
      }
      WARN("Failed to decode image: %s", fullName.c_str());
    }
    else
    {
      WARN("Failed to load image data from blob: %s", fullName.c_str());
    }
    return std::nullopt;
  }

  static sk_sp<SkImage> getSkiaImage(const std::string& name)
  {
    auto tm = getInstance();
    ASSERT(tm);
    if (!tm->contains(name))
    {
      return nullptr;
    }
    return tm->images[name];
  }

  static bool setSkiaImage(const std::string& name, sk_sp<SkImage> img)
  {
    auto tm = getInstance();
    ASSERT(tm);
    if (img)
    {
      tm->images[name] = img;
      return true;
    }
    return false;
  }
};

}; // namespace VGG

#endif // __TEXTURE_MANAGER_HPP__
