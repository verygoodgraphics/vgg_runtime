#pragma once
#include <memory>

class SkCanvas;
class Renderable : public std::enable_shared_from_this<Renderable>
{
public:
  virtual void onRender(SkCanvas* canvas) = 0;
};
