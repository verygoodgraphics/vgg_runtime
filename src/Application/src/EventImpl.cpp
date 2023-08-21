
#include "Log.h"
#include "../../interface/Event/EventAPI.h"

static std::unique_ptr<EventAPI> g_eventImpl = nullptr;

void EventManager::registerEventAPI(std::unique_ptr<EventAPI> impl)
{
  g_eventImpl = std::move(impl);
}

void EventManager::getModState()
{
  ASSERT(g_eventImpl);
}
