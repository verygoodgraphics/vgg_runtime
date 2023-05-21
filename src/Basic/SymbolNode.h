#pragma once
#include "VGGType.h"
#include "PaintNode.h"

namespace VGG
{

class SymbolMasterNode final : public PaintNode
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

  void Paint(SkCanvas* canvas) override
  {
    if (master == nullptr)
    {
      // lazy init
    }
  }

protected:
  void recursivelyRenderPass(SkCanvas* canvas) override
  {
    renderPassBefore();
    if (master)
    {
      master->recursivelyRenderPass(canvas);
    }
    renderPassAfter();
  }
};

} // namespace VGG
