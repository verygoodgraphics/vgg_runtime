#include "environment.h"
#include "NativeExec.hpp"

namespace VGG
{
void Environment::setUp()
{
}

void Environment::tearDown()
{
  auto ins = NativeExec::sharedInstance();
  ins->release();
}

} // namespace VGG