
#include "Layer/AttrSerde.hpp"
#include <core/SkBlendMode.h>
#include <core/SkBlurTypes.h>
#include <core/SkColor.h>
#include <core/SkColorFilter.h>
#include <core/SkImageFilter.h>
#include <core/SkImageInfo.h>
#include <core/SkMatrix.h>
#include <core/SkPaint.h>
#include <core/SkRect.h>
#include <core/SkSamplingOptions.h>
#include <core/SkSurfaceProps.h>
#include <core/SkTileMode.h>
#include <core/SkMaskFilter.h>
#include <src/core/SkBlurMask.h>
#include <src/core/SkMask.h>
#include <effects/SkImageFilters.h>
#include <effects/SkRuntimeEffect.h>
#include <encode/SkPngEncoder.h>
#include <gpu/GrDirectContext.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <core/SkPath.h>
#include <core/SkRRect.h>
#include <optional>
#include <pathops/SkPathOps.h>
#include <src/core/SkRuntimeEffectPriv.h>
#include <limits>
#include <fstream>
#include <string_view>
static sk_sp<SkShader> makeMaskShader(sk_sp<SkImage> mask, const SkMatrix* mat)
{
  auto r = SkRuntimeEffect::MakeForShader(SkString(R"(
        uniform half4 gColor;
		uniform shader child;
		uniform shader mask;
        half4 main(float2 p) {
			return mask.eval(p).a * child.eval(p);
        }
    )"));

  if (!r.effect)
  {
    return nullptr;
  }
  SkRuntimeShaderBuilder builder(std::move(r.effect));
  // builder.uniform("mask") = mask;
  return builder.makeShader(mat);
}

static sk_sp<SkImageFilter> makeBackgroundBlurFilter(sk_sp<SkImage> bg, SkMatrix mat)
{
  SkRuntimeEffect::Options opts;
  SkRuntimeEffectPriv::AllowPrivateAccess(&opts);
  auto r = SkRuntimeEffect::MakeForShader(SkString(R"(
        uniform shader content;
        uniform shader bg;
		uniform float2 size;
		uniform float3x3 scale;
		uniform float3x3 rotate;
        vec4 main(vec2 coord) {
		    float2 bgcoord = (scale*vec3(coord, 1.0)).xy;
            vec4 c = content.eval(coord.xy);
            vec4 b = bg.eval(bgcoord);
			//vec2 n = coord.xy/size.xy;
			vec2 n = bgcoord.xy/size.xy;
			if(n.x <= 1.0 && n.y <= 1.0 && n.x>=0 &&n.y>=0.0){
               //return c*0.3 + b*0.3 + 0.4*vec4(sk_FragCoord.xy/size.xy, 0.0,1.0);
			   return 0.9*b + 0.1*vec4(n ,0,1);
			}
			return vec4(0,0,0,0);
        }
    )"),
                                          opts);
  if (!r.effect)
  {
    return nullptr;
  }
  auto image = SkImageFilters::Image(bg, SkSamplingOptions());
  auto s = SkMatrix::I();
  auto bgContent =
    SkImageFilters::MatrixTransform(SkMatrix::Scale(1, 1), SkSamplingOptions(), image);
  std::string_view childNames[] = { "content", "bg" };
  sk_sp<SkImageFilter> childNodes[] = { nullptr, bgContent };
  SkRuntimeShaderBuilder builder(std::move(r.effect));
  auto b = mat.invert(&mat);
  builder.uniform("rotate") = mat;
  builder.uniform("size") = SkVector{ (float)bg->width(), (float)bg->height() };
  auto blur = SkImageFilters::RuntimeShader(builder, childNames, childNodes, 2);
  return blur;
}
