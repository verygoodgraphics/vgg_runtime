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
#pragma once

#include <rxcpp/rx.hpp>

#include <functional>

namespace VGG
{
class AsyncWorkerFactory
{
public:
  AsyncWorkerFactory() = delete;

  // factory method
  static rxcpp::observe_on_one_worker createTaskWorker()
  {
    return s_task_worker_factory();
  }

  static rxcpp::observe_on_one_worker createResultWorker()
  {
    return s_result_worker_factory();
  }

  // factory setter
  using WorkerFactory = std::function<rxcpp::observe_on_one_worker()>;
  static void setTaskWorkerFactory(WorkerFactory workerFactory)
  {
    s_task_worker_factory = workerFactory;
  }

  static void setResultWorkerFactory(WorkerFactory workerFactory)
  {
    s_result_worker_factory = workerFactory;
  }

private:
  inline static WorkerFactory s_task_worker_factory;
  inline static WorkerFactory s_result_worker_factory;
};

template<class ResultType, class OnError = rxcpp::detail::OnErrorEmpty>
class Async
{
public:
  using TaskFunction = std::function<ResultType()>;
  using OnResult = std::function<void(ResultType)>;

  Async(TaskFunction taskFunction, OnResult onResult, OnError onError = OnError())
    : m_task_function{ std::move(taskFunction) }
    , m_on_result{ std::move(onResult) }
    , m_on_error{ std::move(onError) }
  {
  }

  void operator()()
  {
    auto observable = rxcpp::make_observable_dynamic<ResultType>(
      [&task_funcion = m_task_function](rxcpp::subscriber<ResultType> o)
      {
        try
        {
          auto result = task_funcion();
          o.on_next(std::move(result));
          o.on_completed();
        }
        catch (...)
        {
          o.on_error(std::current_exception());
        }
      });

    observable.subscribe_on(AsyncWorkerFactory::createTaskWorker())
      .observe_on(AsyncWorkerFactory::createResultWorker())
      .subscribe(m_on_result, m_on_error);
  }

private:
  TaskFunction m_task_function;
  OnResult m_on_result;
  OnError m_on_error;
};

} // namespace VGG
