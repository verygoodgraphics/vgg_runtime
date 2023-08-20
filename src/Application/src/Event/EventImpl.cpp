#include "../../interface/Event/EventAPI.h"

#include "Log.h"

static EventImpl* g_eventImpl = nullptr;

void registerEventAPI(EventImpl* impl);
void getModState();

void EventAPI::registerEventAPI(EventImpl* impl)
{
  g_eventImpl = impl;
}

void EventAPI::getModState()
{
  ASSERT(g_eventImpl);
}
