#include "Utility/Log.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include <cstdlib>

namespace VGG::layer
{

class AllocatorImpl : public VGG::layer::VAllocator
{
public:
  int         count = 0;
  const char* name{ nullptr };
  AllocatorImpl(const char* name = nullptr)
    : name(name)
  {
  }
  void dealloc(void* ptr) override
  {
    count--;
    free(ptr);
  }
  void* alloc(size_t size) override
  {
    count++;
    return malloc(size);
  }
  ~AllocatorImpl() override
  {
    if (count > 0)
    {
      WARN("Maybe memory leak in %s, %d VObjects are not released", name, count);
    }
  }
};
static AllocatorImpl g_globalLayerMemoryAllocator{ "Global Memory Allocator" };
// NOLINTBEGIN
VAllocator*          VGG_GlobalMemoryAllocator()
{
  return nullptr;
  // return &g_globalLayerMemoryAllocator;
}
// NOLINTEND

} // namespace VGG::layer
