#pragma once
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

enum BoolOp
{
  BLOP_UNION, // union
  BLOP_SUB,   // substraction
  BLOP_ISECT, // intersection
  BLOP_ECLD,  // exclusion
  BLOP_NONE   // none
};

enum WindingType
{
  WR_NONZERO,
  WR_EVENODD
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

} // namespace VGG
