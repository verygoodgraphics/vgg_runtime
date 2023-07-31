#pragma once
#include "Common/Config.h"
#include "VType.h"
#include "PaintNode.h"

namespace VGG
{

class VGG_EXPORTS SymbolMasterNode final : public PaintNode
{
public:
  std::string symbolID;
  SymbolMasterNode(const std::string& name, std::string guid)
    : PaintNode(name, VGG_MASTER, std::move(guid))
  {
  }
};

class SymbolInstanceNode final : public PaintNode
{
public:
  std::string symbolID;
  std::shared_ptr<SymbolMasterNode> master;
  SymbolInstanceNode(const std::string& name, std::string guid)
    : PaintNode(name, VGG_INSTANCE, std::move(guid))
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
