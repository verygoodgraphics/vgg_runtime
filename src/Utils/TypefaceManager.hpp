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
#ifndef __TYPEFACE_MANAGER_HPP__
#define __TYPEFACE_MANAGER_HPP__

#include <skia/include/core/SkData.h>
#include <skia/include/core/SkStream.h>
#include <skia/include/core/SkTypeface.h>
#include <skia/include/core/SkFontStyle.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <optional>

#include "Utils/Utils.hpp"

namespace VGG
{

struct TypefaceManager
{
  inline static std::unordered_map<std::string, sk_sp<SkTypeface>> typefaces;
  inline static std::unordered_map<std::string, sk_sp<SkTypeface>> typefacesPS;

  inline static std::string getFontStyleDescription(const SkFontStyle& fs)
  {
    using Weight = SkFontStyle::Weight;
    using Width = SkFontStyle::Width;
    using Slant = SkFontStyle::Slant;

    std::string desc;

    switch (fs.weight())
    {
      case Weight::kInvisible_Weight:
        desc += "Invisible";
        break;
      case Weight::kThin_Weight:
        desc += "Thin";
        break;
      case Weight::kExtraLight_Weight:
        desc += "ExtraLight";
        break;
      case Weight::kLight_Weight:
        desc += "Light";
        break;
      case Weight::kNormal_Weight:
        break;
      case Weight::kMedium_Weight:
        desc += "Medium";
        break;
      case Weight::kSemiBold_Weight:
        desc += "SemiBold";
        break;
      case Weight::kBold_Weight:
        desc += "Bold";
        break;
      case Weight::kExtraBold_Weight:
        desc += "ExtraBold";
        break;
      case Weight::kBlack_Weight:
        desc += "Black";
        break;
      case Weight::kExtraBlack_Weight:
        desc += "ExtraBlack";
        break;
    }

    if (desc.empty())
    {
      desc = "Regular";
    }

    switch (fs.width())
    {
      case Width::kUltraCondensed_Width:
        desc += " UltraCondensed";
        break;
      case Width::kExtraCondensed_Width:
        desc += " ExtraCondensed";
        break;
      case Width::kCondensed_Width:
        desc += " Condensed";
        break;
      case Width::kSemiCondensed_Width:
        desc += " SemiCondensed";
        break;
      case Width::kNormal_Width:
        break;
      case Width::kSemiExpanded_Width:
        desc += " SemiExpanded";
        break;
      case Width::kExpanded_Width:
        desc += " Expanded";
        break;
      case Width::kExtraExpanded_Width:
        desc += " ExtraExpanded";
        break;
      case Width::kUltraExpanded_Width:
        desc += " UltraExpanded";
        break;
    }

    switch (fs.slant())
    {
      case Slant::kUpright_Slant:
        break;
      case Slant::kItalic_Slant:
        desc += " Italic";
        break;
      case Slant::kOblique_Slant:
        desc += " Oblique";
        break;
    };

    return desc;
  }

  inline static std::optional<std::string> getTypefaceDescriptor(const sk_sp<SkTypeface> tf)
  {
    ASSERT(tf);
    SkString fm;
    tf->getFamilyName(&fm);
    if (fm.startsWith("."))
    {
      return std::nullopt;
    }

    std::string familyName{ fm.c_str() };
    std::string fontStyleDescription = getFontStyleDescription(tf->fontStyle());

    return familyName + ", " + fontStyleDescription;
  }

  inline static sk_sp<SkTypeface> loadTypefaceFromSerialized(const std::string& data)
  {
    auto stream = SkMemoryStream::Make(SkData::MakeWithCopy(data.data(), data.size()));
    if (auto tf = SkTypeface::MakeDeserialize(stream.get()))
    {
      SkString psName;
      if (tf->getPostScriptName(&psName) && !(psName.isEmpty()))
      {
        typefacesPS[std::string(psName.c_str())] = tf;
      }

      if (auto descOpt = getTypefaceDescriptor(tf))
      {
        auto desc = descOpt.value();
        if (typefaces.find(desc) == typefaces.end())
        {
          typefaces[desc] = tf;
        }
        return typefaces[desc];
      }
    }
    return nullptr;
  }

  inline static sk_sp<SkTypeface> loadTypefaceFromMem(sk_sp<SkData> data, size_t fontIdx = 0)
  {
    ASSERT(data);
    if (auto tf = SkTypeface::MakeFromData(data, fontIdx))
    {
      SkString psName;
      if (tf->getPostScriptName(&psName) && !(psName.isEmpty()))
      {
        typefacesPS[std::string(psName.c_str())] = tf;
      }

      if (auto descOpt = getTypefaceDescriptor(tf))
      {
        auto desc = descOpt.value();
        if (typefaces.find(desc) == typefaces.end())
        {
          typefaces[desc] = tf;
        }
        return typefaces[desc];
      }
    }
    return nullptr;
  }

  inline static size_t loadTypefacesFromMem(sk_sp<SkData> data)
  {
    size_t idx = 0;
    do
    {
      if (auto tf = loadTypefaceFromMem(data, idx))
      {
        idx++;
      }
      else
      {
        break;
      }
    } while (true);
    return idx;
  }

  inline static sk_sp<SkTypeface> loadTypefaceFromFile(const std::string& fp, size_t fontIdx = 0)
  {
    if (auto tf = SkTypeface::MakeFromFile(fp.c_str(), fontIdx))
    {
      SkString psName;
      if (tf->getPostScriptName(&psName) && !(psName.isEmpty()))
      {
        typefacesPS[std::string(psName.c_str())] = tf;
      }

      if (auto descOpt = getTypefaceDescriptor(tf))
      {
        auto desc = descOpt.value();
        if (typefaces.find(desc) == typefaces.end())
        {
          typefaces[desc] = tf;
        }
        return typefaces[desc];
      }
    }
    return nullptr;
  }

  inline static size_t loadTypefacesFromFile(const std::string& fp)
  {
    size_t idx = 0;
    do
    {
      if (auto tf = loadTypefaceFromFile(fp, idx))
      {
        idx++;
      }
      else
      {
        break;
      }
    } while (true);
    return idx;
  }

  inline static std::vector<std::string> getTypefaceList()
  {
    std::vector<std::string> ls;
    for (auto& kv : typefaces)
    {
      ls.push_back(kv.first);
    }
    std::sort(ls.begin(), ls.end());
    return ls;
  }

  inline static sk_sp<SkTypeface> getTypeface(const std::string& desc)
  {
    if (typefaces.find(desc) == typefaces.end())
    {
      return nullptr;
    }
    return typefaces[desc];
  }

  inline static sk_sp<SkTypeface> getTypefaceByPSName(const std::string& psName)
  {
    if (typefacesPS.find(psName) == typefacesPS.end())
    {
      return nullptr;
    }
    return typefacesPS[psName];
  }

  inline static sk_sp<SkData> serializeTypeface(const std::string& desc)
  {
    if (typefaces.find(desc) == typefaces.end())
    {
      return nullptr;
    }
    // TODO minimize ttc fonts by extracting only one typeface from SkTypeface
    return typefaces[desc]->serialize(SkTypeface::SerializeBehavior::kDoIncludeData);
  }
};

}; // namespace VGG

#endif // __TYPEFACE_MANAGER_HPP__
