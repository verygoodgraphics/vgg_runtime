#include "Editor.hpp"

#include <Log.h>

#include <include/core/SkCanvas.h>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

void Editor::handleUIEvent(UIEventPtr event)
{
  DEBUG("Editor::handleUIEvent: type = %s, target = %s",
        event->type().c_str(),
        event->path().c_str());
}

void Editor::onRender(SkCanvas* canvas)
{
  DEBUG("Editor::onRender");
}