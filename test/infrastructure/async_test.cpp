#include "Main/RunLoop.hpp"
#include "Utils/Async.hpp"

#include <gtest/gtest.h>

using namespace VGG;

class VggAsyncTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<RunLoop> m_run_loop;
  bool m_exit_loop = false;

  void SetUp() override
  {
    m_run_loop = std::make_shared<RunLoop>();

    AsyncWorkerFactory::setTaskWorkerFactory([]() { return rxcpp::observe_on_new_thread(); });
    AsyncWorkerFactory::setResultWorkerFactory([run_loop = m_run_loop]()
                                               { return run_loop->thread(); });
  }
  void TearDown() override
  {
  }

  void loop_until_exit()
  {
    while (!m_exit_loop)
    {
      m_run_loop->dispatch();
    }
  }
};

TEST_F(VggAsyncTestSuite, Smoke)
{
  // Given
  auto current_thread_id = std::this_thread::get_id();
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
                  [fake_result, &exit = m_exit_loop, &current_thread_id](int result)
                  {
                    auto result_thread_id = std::this_thread::get_id();
                    // Then
                    EXPECT_EQ(result_thread_id, current_thread_id);
                    EXPECT_EQ(result, fake_result);

                    exit = true;
                  } };

  // When
  sut();

  loop_until_exit();
}

TEST_F(VggAsyncTestSuite, Exception)
{
  // Given
  auto current_thread_id = std::this_thread::get_id();
  constexpr auto fake_result = 1;

  Async<int, std::function<void(std::exception_ptr)>> sut{
    [fake_result, &current_thread_id]()
    {
      auto task_thread_id = std::this_thread::get_id();
      // Then
      EXPECT_NE(task_thread_id, current_thread_id);

      // run task
      std::string().at(1); // this generates a std::out_of_range

      return fake_result;
    },
    [fake_result, &exit = m_exit_loop, &current_thread_id](int result) { GTEST_FAIL(); },
    [&exit = m_exit_loop, &current_thread_id](std::exception_ptr eptr)
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

  loop_until_exit();
}