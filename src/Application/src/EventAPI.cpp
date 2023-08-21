
#include "Event/Keycode.h"
#include "Log.h"
#include "../../interface/Event/EventAPI.h"

static std::unique_ptr<EventAPI> g_vggEventAPIImpl = nullptr;

void EventManager::registerEventAPI(std::unique_ptr<EventAPI> impl)
{
  g_vggEventAPIImpl = std::move(impl);
}

EVGGKeymod EventManager::getModState()
{
  ASSERT_MSG(g_vggEventAPIImpl, "Event API is not initialized");
  return g_vggEventAPIImpl->getModState();
}

uint8_t* EventManager::getKeyboardState(int* nums)
{
  ASSERT_MSG(g_vggEventAPIImpl, "Event API is not initialized");
  return g_vggEventAPIImpl->getKeyboardState(nums);
}
