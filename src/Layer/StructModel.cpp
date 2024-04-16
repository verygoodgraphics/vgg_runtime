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
#include "Layer/Model/StructModel.hpp"
#include "Domain/Model/DesignModel.hpp"
#include "Domain/Model/DesignModelFwd.hpp"
#include "Domain/Model/Element.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Model/Concept.hpp"
#include <variant>

namespace
{
using namespace VGG;
using namespace VGG::layer;
using namespace VGG::Model;

inline void update(
  VGG::Model::TextFontAttributes&       attr,
  const VGG::Model::TextFontAttributes& other)
{

#define M(field)                                                                                   \
  if (other.field)                                                                                 \
  {                                                                                                \
    attr.field = other.field;                                                                      \
  }

  M(baselineShift);
  M(borders);
  // M(textFontAttributesClass);
  M(fills);
  M(fillUseType);
  M(fontVariantCaps);
  M(fontVariantPosition);
  M(fontVariations);
  M(horizontalScale);
  M(hyperlink);
  M(length);
  M(letterSpacingUnit);
  M(letterSpacingValue);
  M(lineSpacingUnit);
  M(linethrough);
  M(name);
  M(postScript);
  M(rotate);
  M(size);
  M(subFamilyName);
  M(textCase);
  M(textParagraph);
  M(underline);
  M(verticalScale);
#undef M
}

template<typename F>
inline std::variant<EModelObjectType, EModelShapeType> subGeometryType(
  const SubGeometryType& c,
  F&&                    f)
{
  if (std::holds_alternative<Model::Contour>(c))
  {
    return { EModelShapeType::CONTOUR };
  }
  else if (std::holds_alternative<Model::Rectangle>(c))
  {
    return { EModelShapeType::RECTANGLE };
  }
  else if (std::holds_alternative<Model::Ellipse>(c))
  {
    return { EModelShapeType::ELLIPSE };
  }
  else if (std::holds_alternative<Model::Polygon>(c))
  {
    return { EModelShapeType::POLYGON };
  }
  else if (std::holds_alternative<Model::Star>(c))
  {
    return { EModelShapeType::STAR };
  }
  else if (std::holds_alternative<Model::VectorNetwork>(c))
  {
    return { EModelShapeType::VECTORNETWORK };
  }
  else if (std::holds_alternative<Model::Frame>(c))
  {
    return EModelObjectType::FRAME;
  }
  else if (std::holds_alternative<Model::Group>(c))
  {
    return { EModelObjectType::GROUP };
  }
  else if (std::holds_alternative<Model::Path>(c))
  {
    return { EModelObjectType::PATH };
  }
  else if (std::holds_alternative<Text>(c))
  {
    return { EModelObjectType::TEXT };
  }
  else if (std::holds_alternative<Image>(c))
  {
    return { EModelObjectType::IMAGE };
  }
  else if (std::holds_alternative<SymbolMaster>(c))
  {
    return { EModelObjectType::MASTER };
  }
  else if (std::holds_alternative<SymbolInstance>(c))
  {
    return { EModelObjectType::INSTANCE };
  }
  return { EModelObjectType::UNKNOWN };
}

template<typename F1, typename F2>
  requires CallableObject<void, F1, EModelObjectType> && CallableObject<void, F2, EModelShapeType>
inline void toType(VGG::Domain::Element* m, F1&& f1, F2&& f2)
{
  using namespace VGG::Domain;
  switch (m->type())
  {
    case Element::EType::FRAME:
      f1(EModelObjectType::FRAME);
      return;
    case Element::EType::GROUP:
      f1(EModelObjectType::GROUP);
      return;
    case Element::EType::PATH:
      f1(EModelObjectType::PATH);
      return;
    case Element::EType::TEXT:
      f1(EModelObjectType::TEXT);
      return;
    case Element::EType::SYMBOL_MASTER:
      f1(EModelObjectType::MASTER);
      return;
    case Element::EType::IMAGE:
      f1(EModelObjectType::IMAGE);
      return;
    case Element::EType::SYMBOL_INSTANCE:
      f1(EModelObjectType::INSTANCE);
      return;
    case Element::EType::CONTOUR:
      f2(EModelShapeType::CONTOUR);
      return;
    case Element::EType::RECTANGLE:
      f2(EModelShapeType::RECTANGLE);
      return;
    case Element::EType::POLYGON:
      f2(EModelShapeType::POLYGON);
      return;
    case Element::EType::STAR:
      f2(EModelShapeType::STAR);
      return;
    case Element::EType::ELLIPSE:
      f2(EModelShapeType::ELLIPSE);
      return;
    case Element::EType::VECTOR_NETWORK:
      f2(EModelShapeType::VECTORNETWORK);
      return;
    case Element::EType::NONE:
    case Element::EType::ROOT:
      DEBUG("not support element type");
      return;
  }
  return;
}

inline StructObject dispatchObject(EModelObjectType modelType, ModelType m)
{
  switch (modelType)
  {
    case EModelObjectType::FRAME:
      return StructFrameObject(std::move(m));
    case EModelObjectType::GROUP:
      return StructGroupObject(std::move(m));
    case EModelObjectType::PATH:
      return StructPathObject(std::move(m));
    case EModelObjectType::TEXT:
      return StructTextObject(std::move(m));
    case EModelObjectType::MASTER:
      return StructMasterObject(std::move(m));
    case EModelObjectType::IMAGE:
      return StructImageObject(std::move(m));
    case EModelObjectType::OBJECT:
      return StructObject(std::move(m), EModelObjectType::OBJECT);
    case EModelObjectType::INSTANCE:
      return StructInstanceObject(std::move(m));
    case EModelObjectType::UNKNOWN:
      break;
  }
  return StructObject(std::move(m), EModelObjectType::UNKNOWN);
}

inline StructObject dispatchObject(EModelShapeType shapeType, ModelType m)
{
  switch (shapeType)
  {
    case EModelShapeType::CONTOUR:
    case EModelShapeType::RECTANGLE:
    case EModelShapeType::ELLIPSE:
    case EModelShapeType::POLYGON:
    case EModelShapeType::STAR:
    case EModelShapeType::VECTORNETWORK:
    case EModelShapeType::UNKNOWN:
      return StructPathObject(m);
  }
  return StructPathObject(0);
}

} // namespace

namespace VGG::layer
{
std::vector<StructObject> StructObject::getChildObjects() const
{
  const auto t = getObjectType();
  switch (t)
  {
    case EModelObjectType::FRAME:
    case EModelObjectType::GROUP:
    case EModelObjectType::MASTER:
    case EModelObjectType::INSTANCE:
    case EModelObjectType::OBJECT:
    {
      std::vector<StructObject> objects;
      for (const auto& c : *m)
      {
        toType(
          c.get(),
          [&](EModelObjectType objectType)
          { objects.emplace_back(dispatchObject(objectType, c.get())); },
          [&](EModelShapeType shapeType)
          { objects.emplace_back(dispatchObject(shapeType, c.get())); });
      }
      return objects;
    }
    case EModelObjectType::PATH:
    case EModelObjectType::IMAGE:
    case EModelObjectType::TEXT:
    case EModelObjectType::UNKNOWN:
      DEBUG("getChildObjects() in class [%d] is useless", (int)t);
      return {};
  }
  return {};
}
//
std::vector<SubShape<StructObject>> StructPathObject::getShapes() const
{
  std::vector<SubShape<StructObject>> res;
  auto                                impl = static_cast<VGG::Domain::PathElement*>(m);
  auto                                model = static_cast<const VGG::Model::Path*>(impl->model());
  const auto&                         shapes = model->shape->subshapes;
  const auto                          bounds = getBounds();
  const auto                          smooth = getCornerSmoothing();

  auto count = impl->childObjects().size();
  ASSERT(count == model->shape->subshapes.size());
  res.reserve(count);
  for (size_t i = 0; i < count; i++)
  {
    const auto& model = shapes[i];
    const auto  child = impl->childObjects()[i];
    const auto  blop = EBoolOp(model.booleanOperation);
    auto        m = child.get();
    toType(
      m,
      [&](EModelObjectType objectType) { res.emplace_back(blop, dispatchObject(objectType, m)); },
      [&](EModelShapeType shapeType)
      {
        std::array<float, 4> radius{ 0, 0, 0, 0 };
        int                  pointCount = 0;
        switch (shapeType)
        {
          case EModelShapeType::RECTANGLE:
          {
            auto dm = static_cast<Domain::RectangleElement*>(m)->dataModel();
            if (dm->radius)
            {
              const auto& r = *dm->radius;
              if (r.size() >= 4)
              {
                radius = { float(r[0]), float(r[1]), float(r[2]), float(r[3]) };
              }
            }
          }
          break;
          case EModelShapeType::POLYGON:
          {
            auto dm = static_cast<Domain::PolygonElement*>(m)->dataModel();
            pointCount = dm->pointCount;
            radius[0] = dm->radius.value_or(0);
          }
          break;
          case EModelShapeType::STAR:
          {
            auto dm = static_cast<Domain::StarElement*>(m)->dataModel();
            pointCount = dm->pointCount;
            radius[0] = dm->radius.value_or(0);
            radius[1] = dm->ratio;
          }
          break;
          case EModelShapeType::VECTORNETWORK:
          case EModelShapeType::CONTOUR:
          case EModelShapeType::ELLIPSE:
          case EModelShapeType::UNKNOWN:
            break;
        }

        res.emplace_back(
          blop,
          makeShapeData2(
            bounds,
            shapeType,
            radius.data(),
            smooth,
            pointCount,
            [&](EModelShapeType type)
            {
              if (type == EModelShapeType::CONTOUR)
              {
                auto       dm = static_cast<Domain::ContourElement*>(m)->dataModel();
                ContourPtr c = std::make_shared<ContourArray>(dm->points.size());
                c->cornerSmooth = smooth;
                c->closed = dm->closed;

                auto toVec2 = [](const std::vector<double>& v) -> glm::vec2 {
                  return v.size() >= 2 ? glm::vec2{ v[0], v[1] } : glm::vec2{ 0, 0 };
                };

                auto toVec2Opt =
                  [](const std::optional<std::vector<double>>& v) -> std::optional<glm::vec2>
                {
                  if (v.has_value())
                    return v->size() >= 2 ? glm::vec2{ (*v)[0], (*v)[1] } : glm::vec2{ 0, 0 };
                  else
                    return std::nullopt;
                };
                for (const auto& e : dm->points)
                {
                  c->emplace_back(
                    toVec2(e.point),
                    e.radius.value_or(0.f),
                    toVec2Opt(e.curveFrom),
                    toVec2Opt(e.curveTo),
                    e.cornerStyle);
                }
                if (!c->closed && !c->empty())
                {
                  c->back().radius = 0;
                  c->front().radius = 0;
                }
                return c;
              }
              return std::make_shared<ContourArray>(4);
            }));
      });
  }
  return res;
}

std::vector<TextStyleAttr> StructTextObject::getOverrideFontAttr() const
{
  const auto& fontAttr = static_cast<const Model::Text*>(m->model())->fontAttr;
  const auto& style = static_cast<const Model::Text*>(m->model())->style;
  const auto& defaultFontAttr = static_cast<const Model::Text*>(m->model())->defaultFontAttr;
  std::vector<TextStyleAttr> textStyle;
  for (const auto& attr : fontAttr)
  {
    auto defaultAttr = *defaultFontAttr;
    update(defaultAttr, attr);
    if (!defaultAttr.fills)
    {
      defaultAttr.fills = style.fills;
    }
    auto a = serde::ModelSerde<Model::TextFontAttributes, TextStyleAttr>::serde_from(defaultAttr);

    textStyle.push_back(a);
  }

  return textStyle;
}

} // namespace VGG::layer
