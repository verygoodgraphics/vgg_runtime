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
#include "Mask.hpp"
#include "StyleItem.hpp"
namespace VGG::layer
{

sk_sp<SkShader> MaskBuilder::makeAlphaMaskShader(
  PaintNode*      self,
  const MaskMap&  maskObjects,
  MaskIter&       iter,
  const SkRect&   rect,
  const SkMatrix* matrix)
{
  auto              components = collectionMasks(self, maskObjects, iter);
  Renderer          alphaMaskRender;
  SkPictureRecorder rec;
  auto              rt = SkRTreeFactory();
  auto              canvas = rec.beginRecording(rect, &rt);
  alphaMaskRender.setCanvas(canvas);
  for (const auto& p : components)
  {
    auto skm = toSkMatrix(p.transform.matrix());
    canvas->save();
    canvas->concat(skm);
    p.mask->styleItem()->renderAsMask(&alphaMaskRender);
    // p.mask->onDrawAsAlphaMask(&alphaMaskRender, 0);
    canvas->restore();
  }
  auto maskShader = SkPictureShader::Make(
    rec.finishRecordingAsPicture(),
    SkTileMode::kClamp,
    SkTileMode::kClamp,
    SkFilterMode::kNearest,
    matrix,
    &rect);
  ASSERT(maskShader);
  return maskShader;
}
} // namespace VGG::layer
