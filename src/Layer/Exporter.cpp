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
#include "Stream.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Exporter/PDFExporter.hpp"
#include "Layer/Exporter/SVGExporter.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Utility/Log.hpp"

#include <core/SkStream.h>
#include <docs/SkPDFDocument.h>
#include <svg/SkSVGCanvas.h>
#include <src/xml/SkXMLWriter.h>
#include <core/SkCanvas.h>

#include <optional>
#include <vector>
#include <ostream>
#include <sstream>
#include <fstream>
#include <iterator>

static void renderInternal(SkCanvas* canvas, VGG::layer::FrameNode* frame)
{
  ASSERT(frame);
  ASSERT(canvas);
  canvas->save();
  canvas->clear(SK_ColorWHITE);
  frame->revalidate();
  // canvas->drawPicture(frame->picture());
  VGG::layer::Renderer r;
  r = r.createNew(canvas);
  frame->render(&r);
  canvas->flush();
  canvas->restore();
}
namespace VGG::layer::exporter
{

void makePDF(layer::FrameNode* frame, const PDFOptions& opts, std::ostream& os)
{
  SkStdOStream skos(os);
  auto         pdfDoc = SkPDF::MakeDocument(&skos);
  if (pdfDoc)
  {
    int       w = opts.extend[0];
    int       h = opts.extend[1];
    SkCanvas* pdfCanvas = pdfDoc->beginPage(w, h);
    if (pdfCanvas)
    {
      renderInternal(pdfCanvas, frame);
    }
    pdfDoc->endPage();
    pdfDoc->close();
  }
  else
  {
    DEBUG("Create PDF Doc failed\n");
  }
}

std::optional<std::vector<char>> makePDF(layer::FrameNode* frame, const PDFOptions& opts)
{
  std::stringstream data;
  layer::exporter::makePDF(frame, opts, data);
  auto d = data.str();
  return std::vector<char>{ d.begin(), d.end() };
}

std::optional<std::vector<char>> makeSVG(layer::FrameNode* frame, const SVGOptions& opts)
{
  std::stringstream data;
  makeSVG(frame, opts, data);
  auto d = data.str();
  return std::vector<char>{ d.begin(), d.end() };
}

void makeSVG(layer::FrameNode* frame, const SVGOptions& opts, std::ostream& os)
{
  auto         rect = SkRect::MakeWH(opts.extend[0], opts.extend[1]);
  SkStdOStream skos(os);
  auto         svgCanvas = SkSVGCanvas::Make(rect, &skos);
  if (svgCanvas)
  {
    renderInternal(svgCanvas.get(), frame);
  }
  else
  {
    DEBUG("Create SVG Canvas failed\n");
  }
}
} // namespace VGG::layer::exporter
