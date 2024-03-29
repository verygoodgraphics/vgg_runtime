#include "Application/RunLoop.hpp"
#include "Application/Async.hpp"

#include <gtest/gtest.h>

using namespace VGG;

class VggAsyncTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<RunLoop> m_runLoop;
  bool                     m_exitLoop = false;

  void SetUp() override
  {
    m_runLoop = RunLoop::sharedInstance();

    AsyncWorkerFactory::setTaskWorkerFactory([]() { return rxcpp::observe_on_new_thread(); });
    AsyncWorkerFactory::setResultWorkerFactory([runLoop = m_runLoop]()
                                               { return runLoop->thread(); });
  }
  void TearDown() override
  {
  }

  void loopUntilExit()
  {
    while (!m_exitLoop)
    {
      m_runLoop->dispatch();
    }
  }
};

TEST_F(VggAsyncTestSuite, Smoke)
{
  // Given
  auto           current_thread_id = std::this_thread::get_id();
  constexpr auto fake_result = 1;

  Async<int> sut{ [fake_result, &current_thread_id]()
                  {
                    auto task_thread_id = std::this_thread::get_id();
                    // Then
                    EXPECT_NE(task_thread_id, current_thread_id);

                    // run task
                    auto result = fake_result;
                    return result;
                  },
                  [fake_result, &exit = m_exitLoop, &current_thread_id](int result)
                  {
                    auto result_thread_id = std::this_thread::get_id();
                    // Then
                    EXPECT_EQ(result_thread_id, current_thread_id);
                    EXPECT_EQ(result, fake_result);

                    exit = true;
                  } };

  // When
  sut();

  loopUntilExit();
}

TEST_F(VggAsyncTestSuite, Exception)
{
  // Given
  auto           current_thread_id = std::this_thread::get_id();
  constexpr auto fake_result = 1;

  Async<int, std::function<void(std::exception_ptr)>> sut{
    [fake_result, &current_thread_id]()
    {
      auto task_thread_id = std::this_thread::get_id();
      // Then
      EXPECT_NE(task_thread_id, current_thread_id);

      // run task
      (void)std::string().at(1); // this generates a std::out_of_range

      return fake_result;
    },
    [fake_result, &exit = m_exitLoop, &current_thread_id](int result) { GTEST_FAIL(); },
    [&exit = m_exitLoop, &current_thread_id](std::exception_ptr eptr)
    {
      // Then
      auto result_thread_id = std::this_thread::get_id();
      EXPECT_EQ(result_thread_id, current_thread_id);

      try
      {
        if (eptr)
          std::rethrow_exception(eptr);
      }
      catch (const std::exception& e)
      {
        std::cout << "Caught exception: '" << e.what() << "'\n";
      }
      catch (...)
      {
        std::cout << "Caught unknown exception. \n";
      }

      exit = true;
    }
  };

  // When
  sut();

  loopUntilExit();
}