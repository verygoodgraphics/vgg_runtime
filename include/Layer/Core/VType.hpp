/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include <cstdint>
#include <sys/types.h>
namespace VGG
{
// TODO:: Could be removed in the future
enum EObjectType
{
  VGG_LAYER = 0,
  VGG_PATH,
  VGG_IMAGE,
  VGG_TEXT,
  VGG_ARTBOARD,
  VGG_GROUP,
  VGG_FRAME,
  VGG_CONTOUR,
  VGG_MASTER,
  VGG_INSTANCE
};

enum EBoolOp
{
  BO_UNION,        // union
  BO_SUBSTRACTION, // substraction
  BO_INTERSECTION, // intersection
  BO_EXECLUSION,   // exclusion
  BO_NONE          // none
};

enum EWindingType
{
  WR_NONE_ZERO,
  WR_EVEN_ODD
};

enum EOverflow
{
  OF_HIDDEN = 1,
  OF_VISIBLE = 2,
  OF_SCROLL = 3
};

enum EBlendMode
{
  // common in ai, sketch, octopus
  BM_NORMAL,
  BM_DARKEN,
  BM_MULTIPLY,
  BM_COLOR_BURN,
  BM_LIGHTEN,
  BM_SCREEN,
  BM_COLOR_DODGE,
  BM_OVERLAY,
  BM_SOFT_LIGHT,
  BM_HARD_LIGHT,
  BM_DIFFERENCE,
  BM_EXCLUSION,
  BM_HUE,
  BM_SATURATION,
  BM_COLOR,
  BM_LUMINOSITY,

  // sketch
  BM_PLUS_DARKER,
  BM_PlUS_LIGHTER,

  //  octopus
  BM_BLEND_DIVIDE,
  BM_BLEND_SUBSTRACTION,
  BM_DARKER_COLOR,
  BM_DISSOLVE,
  BM_HARD_MIX,
  BM_LIGHTER_COLOR,
  BM_LIGHTEN_BURN,
  BM_LIGHTEN_DODGE,
  BM_LIGHTEN_LIGHT,
  BM_PASS_THROUGHT,
  BM_PIN_LIGHT,
  BM_VIVID_LIGHT,
};

enum EPathFillType
{
  FT_COLOR = 0,
  FT_GRADIENT,
  FT_PATTERN
};

enum ETextLayoutMode
{
  TL_FIXED,
  TL_AUTOWIDTH,
  TL_AUTOHEIGHT
};

enum ETextVerticalAlignment
{
  VA_TOP,
  VA_CENTER,
  VA_BOTTOM,
};

enum ETextHorizontalAlignment
{
  HA_LEFT,
  HA_RIGHT,
  HA_CENTER,
  HA_JUSTIFY,
  HA_NATURAL
};

enum ETextUnderline : uint8_t
{
  UT_NONE,
  UT_SINGLE,
  UT_DOUBLE
};

enum EMaskType : uint8_t
{
  MT_NONE,
  MT_OUTLINE,
  MT_ALPHA
};

enum EMaskShowType : uint8_t
{
  MST_CONTENT = 0,
  MST_BOUNDS = 1,
  MST_INVISIBLE = 2
};

enum ECoutourType : uint8_t
{
  MCT_FRAMEONLY, // not recursive
  MCT_UNION,
  MCT_INTERSECT,
  MCT_UNION_WITH_FRAME,
  MCT_INTERSECT_WITH_FRAME,
  MCT_OBJECT_OPS
};

enum ELineJoin : uint8_t
{
  LJ_MITER,
  LJ_ROUND,
  LJ_BEVEL,
};

enum ELineCap : uint8_t
{
  LC_BUTT,
  LC_ROUND,
  LC_SQUARE,
};

enum EPathPosition : uint8_t
{
  PP_CENTER,
  PP_INSIDE,
  PP_OUTSIDE,
};

enum EGradientType : uint8_t
{
  GT_LINEAR,
  GT_RADIAL,
  GT_ANGULAR
};

enum EPointMode : uint8_t
{
  PM_STRAIGHT = 1,
  PM_MIRRORED = 2,
  PM_ASYMMETRIC = 3,
  PM_DISCONNECTED = 4,
};

enum EImageFillType : uint8_t
{
  IFT_TITL = 0,
  IFT_FILL = 1,
  IFT_STRETCH = 2,
  IFT_FIT = 3,
  IFT_HORIZONTAL_TILE_ONLY = 4,
  IFT_VERTICAL_TILE_ONLY = 5
};

enum ETilePatternType : uint8_t
{
  TILE_BOTH = 0,
  TILE_HORIZONTAL = 1,
  TILE_VERTICAL = 2,
};

enum EFillModeType : uint8_t
{
  FILL_FILL = 0,
  FILL_FIT = 1
};

enum EKnockoutType : uint8_t
{
  KT_OFF = 0,
  KT_ON = 1,
  KT_NEUTRUAL = 2
};

enum EBlurType : uint8_t
{
  BT_LAYER = 0,
  BT_MOTION = 1,
  BT_RADIAL = 2,
  BT_BACKGROUND = 3
};

enum EAlphaMaskType
{
  AM_ALPHA,
  AM_LUMINOSITY,
  AM_INVERSE_LUMINOSITY
};

enum ETextLineType : uint8_t
{
  TLT_PLAIN = 0,
  TLT_ORDERED = 1,
  TLT_UNORDERED = 2,
};

enum ELetterTransform : uint8_t
{
  ELT_NOTHING = 0,
  ELT_SMALL_CAPS,
  ELT_UPPER_CAPS,
  ELT_LOWER_CAPS,
  ELT_FORCE_SMALL_CAPS,
  ELT_TITLE
};

} // namespace VGG
