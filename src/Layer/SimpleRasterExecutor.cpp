/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "SimpleRasterExecutor.hpp"

#include <gpu/GrRecordingContext.h>

namespace VGG::layer
{

RasterManager::RasterResult::Future SimpleRasterExecutor::addRasterTask(
  std::unique_ptr<RasterManager::RasterTask> rasterTask)
{
  using RR = RasterManager::RasterResult;
  const auto task =
    std::make_shared<std::packaged_task<RR()>>([&]() { return rasterTask->execute(context()); });
  add([task]() { (*task)(); });
  return task->get_future();
}

} // namespace VGG::layer
