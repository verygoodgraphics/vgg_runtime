/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Model/Concept.hpp"
#include "Layer/Model/ModelUtils.hpp"
#include "Domain/Model/DesignModel.hpp"
#include "Layer/Model/ModelSerde.hpp"

#include "Layer/Model/StructModelSerde.hpp"
#include <type_traits>

#define M_OBJECT_FIELD_DEF(classtype, req, key, type)                                              \
  type get##req() const                                                                            \
  {                                                                                                \
    using FromType = std::remove_cvref_t<decltype(static_cast<classtype*>(m.get())->key)>;         \
    return serde::model_serde_from<FromType, type, serde::ModelSerde<FromType, type>>(             \
      static_cast<classtype*>(m.get())->key);                                                      \
  }

#define M_OBJECT_DFT_FIELD(classtype, req, key, type, dft)                                         \
  type get##req() const                                                                            \
  {                                                                                                \
    using FromType = std::remove_cvref_t<decltype(*static_cast<classtype*>(m.get())->key)>;        \
    if (static_cast<classtype*>(m.get())->key)                                                     \
    {                                                                                              \
      return serde::model_serde_from<FromType, type, serde::ModelSerde<FromType, type>>(           \
        (*(static_cast<classtype*>(m.get())->key)));                                               \
    }                                                                                              \
    return dft;                                                                                    \
  }

#define M_OBJECT_OPT_FIELD(classtype, req, key, type, dft)                                         \
  type get##req() const                                                                            \
  {                                                                                                \
    using FromType = decltype(*(static_cast<classtype*>(m.get())->key));                           \
    if (static_cast<classtype*>(m.get())->key)                                                     \
    {                                                                                              \
      return serde::model_serde_from<FromType, type, serde::ModelSerde<FromType, type>>(           \
        (*(static_cast<classtype*>(m.get())->key)));                                               \
    }                                                                                              \
    return dft;                                                                                    \
  }

/*
#define M_OBJECT_FIELD_DEF(classtype, req, key, type)                                              \
  type get##req() const                                                                            \
  {                                                                                                \
    return {};                                                                                     \
  }

#define M_OBJECT_DFT_FIELD(classtype, req, key, type, dft)                                     \
  type get##req() const                                                                            \
  {                                                                                                \
    return {};                                                                                     \
  }
  */

namespace VGG::layer
{
using ModelType = std::unique_ptr<VGG::Model::Object>;

struct StructObject
{
  DECL_MODEL_OBJECT(StructObject, StructObject);

  ModelType        m;
  EModelObjectType type;
  StructObject(ModelType m, EModelObjectType t)
    : m(std::move(m))
    , type(t)
  {
  }
  StructObject(const StructObject& other)
  {
    m = std::make_unique<VGG::Model::Object>(*other.m);
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::Object, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::Object, req, key, type, dft)
  EModelObjectType getObjectType() const
  {
    return type;
  }
  R_OPT(Name, name, std::string, "");
  R(Id, id, std::string);
  R(Bounds, bounds, Bounds);
  R(Matrix, matrix, glm::mat3);
  R(Visible, visible, bool);
  R(Overflow, overflow, EOverflow);
  R(Style, style, Style);
  R(ContextSetting, contextSettings, ContextSetting);
  R_OPT(CornerSmoothing, cornerSmoothing, float, 0.f);
  R(MaskType, maskType, EMaskType);
  R_OPT(MaskShowType, maskShowType, EMaskShowType, EMaskShowType::MST_BOUNDS);
  R(ShapeMask, outlineMaskBy, std::vector<std::string>);
  R(AlphaMask, alphaMaskBy, std::vector<AlphaMask>);
  std::vector<StructObject> getChildObjects() const;
#undef R_OPT
#undef R
};

struct StructImageObject : public StructObject
{
  DECL_MODEL_IMAGE(StructImageObject, StructObject);
  StructImageObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::IMAGE)
  {
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::Image, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::Image, req, key, type, dft)
  R(ImageBounds, bounds, Bounds);
  R(ImageGUID, imageFileName, std::string);
  R_OPT(ImageFilter, imageFilters, ImageFilter, ImageFilter{});
#undef R_OPT
#undef R
};

struct StructTextObject : public StructObject
{
  DECL_MODEL_TEXT(StructTextObject, StructObject);
  StructTextObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::TEXT)
  {
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::Text, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::Text, req, key, type, dft)

  R(Text, content, std::string);
  R(TextBounds, bounds, Bounds);
  R(LayoutMode, frameMode, ETextLayoutMode);
  R(VerticalAlignment, verticalAlignment, ETextVerticalAlignment);
  R_OPT(Anchor, anchorPoint, Float2, (Float2{ 0, 0 }));
  R_OPT(TextLineType, textLineType, std::vector<TextLineAttr>, {});
  R_OPT(DefaultFontAttr, defaultFontAttr, TextStyleAttr, {});
  R(FontAttr, fontAttr, std::vector<TextStyleAttr>);
  R(HorizontalAlignment, horizontalAlignment, std::vector<ETextHorizontalAlignment>);
#undef R_OPT
#undef R
};

struct StructGroupObject : public StructObject
{
  DECL_MODEL_GROUP(StructGroupObject, StructObject);
  StructGroupObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::GROUP)
  {
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::Group, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::Group, req, key, type, dft)
#undef R_OPT
#undef R
};

struct StructFrameObject : public StructObject
{
  DECL_MODEL_FRAME(StructFrameObject, StructObject);
  StructFrameObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::FRAME)
  {
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::Frame, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::Frame, req, key, type, dft)
  R_OPT(Radius, radius, Float4, (Float4{ 0, 0, 0, 0 }));
#undef R_OPT
#undef R
};

struct StructMasterObject : public StructObject
{
  DECL_MODEL_MASTER(StructMasterObject, StructObject);
  StructMasterObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::MASTER)
  {
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::SymbolMaster, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::SymbolMaster, req, key, type, dft)

  R_OPT(Radius, radius, Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
#undef R_OPT
#undef R
};

struct StructPathObject : public StructObject
{
  DECL_MODEL_PATH(StructPathObject, StructObject);
  StructPathObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::PATH)
  {
  }

#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::Path, req, key, type)
#define R_OPT(req, key, type, dft) M_OBJECT_DFT_FIELD(VGG::Model::Path, req, key, type, dft)
  EWindingType getWindingType() const
  {
    return EWindingType();
  }
  std::vector<SubShape<StructObject>> getShapes() const;
#undef R_OPT
#undef R
};

struct StructInstanceObject : public StructObject
{
  DECL_MODEL_INSTANCE(StructInstanceObject, StructObject);
  StructInstanceObject(ModelType m)
    : StructObject(std::move(m), EModelObjectType::INSTANCE)
  {
  }
#define R(req, key, type) M_OBJECT_FIELD_DEF(VGG::Model::SymbolInstance, req, key, type)
#define R_OPT(req, key, type, dft)                                                                 \
  M_OBJECT_DFT_FIELD(VGG::Model::SymbolInstance, req, key, type, dft)

#undef R_OPT
#undef R
};

struct StructModelCastObject
{
  static StructGroupObject asGroup(StructObject o)
  {
    return StructGroupObject(std::move(o.m));
  }

  static StructFrameObject asFrame(StructObject o)
  {
    return StructFrameObject(std::move(o.m));
  }

  static StructImageObject asImage(StructObject o)
  {
    return StructImageObject(std::move(o.m));
  }

  static StructTextObject asText(StructObject o)
  {
    return StructTextObject(std::move(o.m));
  }

  static StructMasterObject asMaster(StructObject o)
  {
    return StructMasterObject(std::move(o.m));
  }

  static StructPathObject asPath(StructObject o)
  {
    return StructPathObject(std::move(o.m));
  }

  static StructInstanceObject asInstance(StructObject o)
  {
    return StructInstanceObject(std::move(o.m));
  }
};

using StructModelFrame = ModelPolicy<StructFrameObject, StructModelCastObject>;
#undef M_OBJECT_FIELD_DEF

} // namespace VGG::layer
