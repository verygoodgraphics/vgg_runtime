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
#include "Layer/Model/Concept.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/ImageNode.hpp"
#include "Layer/Model/JSONModel.hpp"
#include "Layer/CoordinateConvert.hpp"

#include <nlohmann/json.hpp>
#include <functional>

namespace VGG::layer
{

struct Serde
{
  using FontNameVisitor =
    std::function<void(const std::string& familyName, const std::string& subFamilyName)>;

  struct Context
  {
    VAllocator*     alloc;
    FontNameVisitor fontNameVisitor;
  };

  static std::tuple<glm::mat3, glm::mat3, glm::mat3> makeMatrix(const glm::mat3& m)
  {
    const auto [newMatrix, inversed] = CoordinateConvert::convertMatrixCoordinate(m);
    return { m, newMatrix, inversed };
  }

  template<typename T, typename F1, typename F2>
    requires AbstractObject<T> && CallableObject<PaintNode*, F1, std::string, std::string> &&
             CallableObject<void, F2, PaintNode*, glm::mat3, Bounds>
  static PaintNodePtr makeObjectBase(
    const T&         m,
    const glm::mat3& totalMatrix,
    F1&&             creator,
    F2&&             override)
  {
    auto obj = creator(m.getName(), m.getId());
    if (!obj)
      return nullptr;

    auto [originalMatrix, newMatrix, inversedNewMatrix] = Serde::makeMatrix(m.getMatrix());
    const auto convertedMatrix = inversedNewMatrix * totalMatrix * originalMatrix;

    auto bounds = m.getBounds();
    CoordinateConvert::convertCoordinateSystem(bounds, convertedMatrix);

    auto style = m.getStyle();
    CoordinateConvert::convertCoordinateSystem(style, convertedMatrix);

    obj->setTransform(Transform(newMatrix));
    obj->setFrameBounds(bounds);
    obj->setStyle(style);
    obj->setFrameCornerSmoothing(m.getCornerSmoothing());
    obj->setContextSettings(m.getContextSetting());
    obj->setMaskBy(m.getShapeMask());
    obj->setAlphaMaskBy(m.getAlphaMask());

    const auto maskType = m.getMaskType();
    obj->setMaskType(maskType);
    const auto defaultShowType =
      maskType == EMaskType::MT_OUTLINE
        ? MST_CONTENT
        : (maskType == EMaskType::MT_ALPHA && false ? MST_BOUNDS : MST_INVISIBLE);
    auto maskShowType = m.getMaskShowType();
    obj->setMaskShowType(maskShowType ? *maskShowType : defaultShowType); // default show type??
    obj->setOverflow(m.getOverflow());
    obj->setVisible(m.getVisible());
    override(obj, convertedMatrix, bounds);
    return obj;
  }

  template<typename T, typename C>
    requires layer::GroupObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(
          ctx.alloc,
          std::move(name),
          VGG_GROUP,
          std::move(guid),
          ERenderTraitBits::RT_RENDER_CHILDREN);
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setOverflow(OF_VISIBLE); // Group do not clip inner content
        p->setContourOption(ContourOption(ECoutourType::MCT_UNION, false));
        const auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(Serde::dispatchObject<typename T::BaseType, C>(c, matrix, ctx));
        }
      });
  }

  template<typename T, typename C>
    requires layer::FrameObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p =
          makePaintNodePtr(ctx.alloc, std::move(name), VGG_FRAME, std::move(guid), RT_DEFAULT);
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setContourOption(ContourOption(ECoutourType::MCT_FRAMEONLY, false));
        p->setFrameRadius(m.getRadius());
        auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(Serde::dispatchObject<typename T::BaseType, C>(c, matrix, ctx));
        }
      });
  }
  template<typename T, typename C>
    requires layer::PathObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(
          ctx.alloc,
          std::move(name),
          VGG_PATH,
          std::move(guid),
          ERenderTraitBits::RT_RENDER_SELF);
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
      {
        p->setChildWindingType(m.getWindingType());
        p->setContourOption(ContourOption(ECoutourType::MCT_OBJECT_OPS, false));
        auto shapes = m.getShapes();
        for (auto& subshape : shapes)
        {
          const auto blop = subshape.booleanOperation;
          auto&      geo = subshape.geometry;
#define VT(var, type, v)                                                                           \
  auto v = std::get_if<type>(var);                                                                 \
  v
          if (VT(&geo, typename T::BaseType, ptr))
          {
            // none shape child
            p->addSubShape(dispatchObject<typename T::BaseType, C>(*ptr, totalMatrix, ctx), blop);
          }
          else if (VT(&geo, ShapeData, ptr))
          {
            auto node =
              makePaintNodePtr(ctx.alloc, "contour", VGG_CONTOUR, "", ERenderTraitBits::RT_DEFAULT);
            node->setOverflow(OF_VISIBLE);
            node->setContourOption(ContourOption{ ECoutourType::MCT_FRAMEONLY, false });
            CoordinateConvert::convertCoordinateSystem(*ptr, totalMatrix);
            node->setContourData(*ptr);
            p->addSubShape(node, blop);
          }
#undef VT
        }
      });
  }

  template<typename T, typename C>
    requires layer::FrameObject<T> && layer::CastObject<C, T>
  static std::vector<PaintNodePtr> from(
    const std::vector<T>& frames,
    const glm::mat3&      matrix,
    const Context&        ctx)
  {
    std::vector<PaintNodePtr> res;
    for (const auto& f : frames)
    {
      res.push_back(Serde::from<T, C>(f, matrix, ctx));
    }
    return res;
  }

  template<typename T, typename C>
    requires layer::MasterObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(
          ctx.alloc,
          std::move(name),
          VGG_FRAME,
          std::move(guid),
          ERenderTraitBits::RT_DEFAULT);
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setContourOption(ContourOption(ECoutourType::MCT_FRAMEONLY, false));
        p->setFrameRadius(m.getRadius());
        auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(Serde::dispatchObject<typename T::BaseType, C>(c, matrix, ctx));
        }
      });
  }

  template<typename T, typename C>
    requires layer::InstanceObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      {
        auto p = makePaintNodePtr(
          ctx.alloc,
          std::move(name),
          VGG_FRAME,
          std::move(guid),
          ERenderTraitBits::RT_DEFAULT);
        return p;
      },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bound)
      {
        p->setContourOption(ContourOption(ECoutourType::MCT_FRAMEONLY, false));
        p->setFrameRadius(m.getRadius());
        auto childObjects = m.getChildObjects();
        for (const auto& c : childObjects)
        {
          p->addChild(Serde::dispatchObject<typename T::BaseType, C>(c, matrix, ctx));
        }
      });
  }

  template<typename T, typename C>
    requires layer::ImageObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      { return makeImageNodePtr(ctx.alloc, std::move(name), std::move(guid)); },
      [&](PaintNode* p, const glm::mat3& matrix, const Bounds& bounds)
      {
        auto i = static_cast<ImageNode*>(p);
        i->setImageBounds(bounds);
        i->setImage(m.getImageGUID());
        i->setImageFilter(m.getImageFilter());
        // i->setReplacesImage(j.value("fillReplacesImage", false));
      });
  }

  template<typename T, typename C>
    requires layer::TextObject<T> && layer::CastObject<C, T>
  static PaintNodePtr from(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    return makeObjectBase(
      m,
      totalMatrix,
      [&](std::string name, std::string guid)
      { return makeTextNodePtr(ctx.alloc, std::move(name), std::move(guid)); },
      [&](PaintNode* ptr, const glm::mat3& matrix, const Bounds& bounds)
      {
        auto p = static_cast<TextNode*>(ptr);
        p->setVerticalAlignment(m.getVerticalAlignment());
        p->setFrameMode(m.getLayoutMode());
        if (auto anchor = m.getAnchor(); anchor)
        {
          glm::vec2 anchorPoint = { (*anchor)[0], (*anchor)[1] };
          CoordinateConvert::convertCoordinateSystem(anchorPoint, totalMatrix);
          p->setTextAnchor(anchorPoint);
        }
        p->setParagraphBounds(bounds);
        // 1.
        std::vector<TextStyleAttr> textStyle = m.getOverrideFontAttr();
        for (auto& style : textStyle)
        {
          CoordinateConvert::convertCoordinateSystem(style, totalMatrix);
        }

        // 2.
        auto lineType = m.getTextLineType();
        auto alignments = m.getHorizontalAlignment();

        std::vector<ParagraphAttr> parStyle;
        parStyle.reserve(lineType.size());
        size_t       i = 0;
        auto         defaultAlign = alignments.empty() ? HA_LEFT : alignments.back();
        TextLineAttr defaultLineType;
        while (i < lineType.size() && i < alignments.size())
        {
          parStyle.emplace_back(lineType[i], alignments[i]);
          i++;
        }
        while (i < lineType.size())
        {
          parStyle.emplace_back(lineType[i], defaultAlign);
          i++;
        }
        while (i < alignments.size())
        {
          parStyle.emplace_back(defaultLineType, alignments[i]);
          i++;
        }
        if (ctx.fontNameVisitor)
        {
          for (const auto& style : textStyle)
          {
            ctx.fontNameVisitor(style.font.fontName, style.font.subFamilyName);
          }
        }
        p->setParagraph(m.getText(), std::move(textStyle), std::move(parStyle));
        if (bounds.width() == 0 || bounds.height() == 0)
        {
          p->setFrameMode(TL_AUTOWIDTH);
        }
        return p;
      });
  }

  template<typename M>
  static PaintNodePtr from(
    const typename M::Model& m,
    const glm::mat3&         totalMatrix,
    const Context&           ctx)
  {
    return from<typename M::Model, typename M::CastObject>(m, totalMatrix, ctx);
  }

  template<typename M>
  static std::vector<PaintNodePtr> from(
    const std::vector<typename M::Model>& models,
    const glm::mat3&                      matrix,
    const Context&                        ctx)
  {
    std::vector<PaintNodePtr> nodes;
    for (const auto& m : models)
    {
      nodes.push_back(Serde::from<M>(m, matrix, ctx));
    }
    return nodes;
  }

private:
  template<typename T, typename C>
    requires layer::AbstractObject<T> && CastObject<C, T>
  static PaintNodePtr dispatchObject(const T& m, const glm::mat3& totalMatrix, const Context& ctx)
  {
    PaintNodePtr ro;
    switch (m.getObjectType())
    {
      case EModelObjectType::OBJECT:
        return nullptr;
      case EModelObjectType::GROUP:
        return Serde::from<decltype(C::asGroup(m)), C>(C::asGroup(m), totalMatrix, ctx);
      case EModelObjectType::FRAME:
        return Serde::from<decltype(C::asFrame(m)), C>(C::asFrame(m), totalMatrix, ctx);
      case EModelObjectType::PATH:
        return Serde::from<decltype(C::asPath(m)), C>(C::asPath(m), totalMatrix, ctx);
      case EModelObjectType::IMAGE:
        return Serde::from<decltype(C::asImage(m)), C>(C::asImage(m), totalMatrix, ctx);
      case EModelObjectType::TEXT:
        return Serde::from<decltype(C::asText(m)), C>(C::asText(m), totalMatrix, ctx);
      case EModelObjectType::MASTER:
        return Serde::from<decltype(C::asMaster(m)), C>(C::asMaster(m), totalMatrix, ctx);
        break;
      case EModelObjectType::INSTANCE:
        return Serde::from<decltype(C::asInstance(m)), C>(C::asInstance(m), totalMatrix, ctx);
      case EModelObjectType::UNKNOWN:
        DEBUG("unknown object type");
        break;
    }
    return nullptr;
  }
};
} // namespace VGG::layer
