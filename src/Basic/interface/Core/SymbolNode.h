#pragma once
#include "Common/Config.h"
#include "VGGType.h"
#include "PaintNode.h"

namespace VGG
{

class VGG_EXPORTS SymbolMasterNode final : public PaintNode
{
public:
  std::string symbolID;
  SymbolMasterNode(const std::string& name)
    : PaintNode(name, VGG_MASTER)
  {
  }
};

class SymbolInstanceNode final : public PaintNode
{
public:
  std::string symbolID;
  std::shared_ptr<SymbolMasterNode> master;
  SymbolInstanceNode(const std::string& name)
    : PaintNode(name, VGG_INSTANCE)
  {
  }

  void paintEvent(SkCanvas* canvas) override
  {
    if (master == nullptr)
    {
      // lazy init
    }
  }

protected:
  void renderOrderPass(SkCanvas* canvas) override
  {
    if (master)
    {
      master->invokeRenderPass(canvas);
    }
  }
};

} // namespace VGG
