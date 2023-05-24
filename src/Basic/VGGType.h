#pragma once
#include <cstdint>
namespace VGG
{

enum ObjectType
{
  VGG_LAYER = 0,
  VGG_PATH,
  VGG_IMAGE,
  VGG_TEXT,
  VGG_ARTBOARD,
  VGG_GROUP,
  VGG_CONTOUR,
  VGG_MASTER,
  VGG_INSTANCE
};

enum EBoolOp
{
  BO_Union,        // union
  BO_Substraction, // substraction
  BO_Intersection, // intersection
  BO_Exclusion,    // exclusion
  BO_None          // none
};

enum EWindingType
{
  WR_NoneZero,
  WR_EvenOdd
};

enum EBlendMode
{
  // common in ai, sketch, octopus
  BM_Normal,
  BM_Darken,
  BM_Multiply,
  BM_Color_burn,
  BM_Lighten,
  BM_Screen,
  BM_Color_dodge,
  BM_Overlay,
  BM_Soft_light,
  BM_Hard_light,
  BM_Difference,
  BM_Exclusion,
  BM_Hue,
  BM_Saturation,
  BM_Color,
  BM_Luminosity,

  // sketch
  BM_Plus_darker,
  BM_Plus_lighter,

  //  octopus
  BM_Blend_divide,
  BM_Blend_subtraction,
  BM_Darker_color,
  BM_Dissolve,
  BM_Hard_mix,
  BM_Lighter_color,
  BM_Lighten_burn,
  BM_Lighten_dodge,
  BM_Lighten_light,
  BM_Pass_through,
  BM_Pin_Light,
  BM_Vivid_light,
};

enum EPathFillType
{
  FT_Color = 0,
  FT_Gradient,
  FT_Pattern
};

enum ETextLayoutMode
{
  TL_Fixed,
  TL_WidthAuto,
  TL_HeightAuto
};

enum ETextVerticalAlignment
{
  VA_Top,
  VA_Center,
  VA_Bottom,
};

enum ETextHorizontalAlignment
{
  HA_Left,
  HA_Right,
  HA_Center,
  HA_Justify,
  HA_Natural
};

enum ETextUnderline : uint8_t
{
  UT_None,
  UT_Single,
  UT_Double
};

enum EMaskType : uint8_t
{
  MT_None,
  MT_Outline,
  MT_Alpha
};

enum ELineJoin : uint8_t
{
  LJ_Miter,
  LJ_Round,
  LJ_Bevel,
};

enum ELineCap : uint8_t
{
  LC_Butt,
  LC_Round,
  LC_Square,
};

enum EPathPosition : uint8_t
{
  PP_Center,
  PP_Inside,
  PP_Outside,
};

enum EGradientType : uint8_t
{
  GT_Linear,
  GT_Radial,
  GT_Angular
};

enum EPointMode
{
  PM_Straight = 1,
  PM_Mirrored = 2,
  PM_Asymmetric = 3,
  PM_Disconnected = 4,
};

enum EImageFillType
{
  IFT_Tile,
  IFT_Fill,
  IFT_Stretch,
  IFT_Fit,
  IFT_OnlyTileHorizontal,
  IFT_OnlyTileVertical
};

} // namespace VGG
