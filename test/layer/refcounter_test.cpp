#include "Layer/Memory/VRefCnt.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include "Layer/Memory/VObject.hpp"
#include "Layer/Memory/ObjectImpl.hpp"
#include "Layer/Memory/RefCountedObjectImpl.hpp"
#include "Layer/Memory/RefCounterImpl.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Memory/Ref.hpp"
#include "Layer/Core/Timer.hpp"

#include <cstdlib>
#include <gtest/gtest.h>
#include <condition_variable>
#include <random>
#include <queue>
#include <future>
#include <thread>
#include <memory>
using namespace VGG::layer;

using namespace std;

// NOLINTBEGIN
struct ThreadPool
{
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(size_t);
  ~ThreadPool();
  template<typename F, typename... Args>
  auto appendTask(F&& f, Args&&... args);
  void wait();

private:
  vector<thread>          m_workers;
  queue<function<void()>> m_tasks;
  mutex                   m_mut;
  atomic<size_t>          m_idle;
  condition_variable      m_cond;
  condition_variable      m_waitCond;
  size_t                  m_nthreads;
  bool                    m_stop;
};

inline ThreadPool::ThreadPool(size_t threads)
  : m_idle(threads)
  , m_nthreads(threads)
  , m_stop(false)
{
  for (size_t i = 0; i < threads; ++i)
    m_workers.emplace_back(
      [this]
      {
        while (true)
        {
          function<void()> task;
          {
            unique_lock<mutex> lock(this->m_mut);
            this->m_cond.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });
            if (this->m_stop)
            {
              return;
            }
            m_idle--;
            task = std::move(this->m_tasks.front());
            this->m_tasks.pop();
          }
          task();
          m_idle++;
          {
            lock_guard<mutex> lk(this->m_mut);
            if (m_idle.load() == this->m_nthreads && this->m_tasks.empty())
            {
              m_waitCond.notify_all();
            }
          }
        }
      });
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::appendTask(F&& f, Args&&... args)
{
  using return_type = invoke_result_t<F, Args...>;

  // perfect capture couble be used in C++20.
  auto task = make_shared<packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  future<return_type> res = task->get_future();
  {
    unique_lock<mutex> lock(m_mut);
    // don't allow enqueueing after stopping the pool
    if (m_stop)
    {
      throw runtime_error("enqueue on stopped ThreadPool");
    }
    m_tasks.emplace([task]() { (*task)(); });
  }
  m_cond.notify_one();
  return res;
}

inline void ThreadPool::wait()
{
  mutex              m;
  unique_lock<mutex> l(m);
  m_waitCond.wait(l, [this]() { return this->m_idle.load() == m_nthreads && m_tasks.empty(); });
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
  {
    unique_lock<mutex> lock(m_mut);
    m_stop = true;
  }
  m_cond.notify_all();
  for (thread& worker : m_workers)
  {
    if (worker.joinable())
      worker.join();
  }
}

// NOLINTEND

class TestAllocator : public VAllocator
{
  std::atomic_int m_count{ 0 };

public:
  virtual void free(void* ptr) override
  {
    free(ptr);
    m_count--;
  }
  virtual void* alloc(size_t size) override
  {
    m_count++;
    return malloc(size);
  }

  void check()
  {
    ASSERT_EQ(m_count.load(), 0);
  }

  ~TestAllocator()
  {
    check();
  }
};

static TestAllocator g_alloc;

class TestClassA : public ObjectImpl<VObject>
{
  int m_num{ 0 };

public:
  TestClassA(VRefCnt* cnt, int num)
    : ObjectImpl<VObject>(cnt)
    , m_num(num)
  {
  }
};

TEST(Memory, ref_new_without_alloc)
{
  auto a = V_NEW<TestClassA>(0);
  ASSERT_EQ(a->refCnt()->refCount(), 1);
  a->ref();
  ASSERT_EQ(a->refCnt()->refCount(), 2);
  a->deref();
  ASSERT_EQ(a->refCnt()->refCount(), 1);
  a->deref();
  ASSERT_EQ(a->refCnt()->refCount(), 0);
}

TEST(Memory, ref_wrapper)
{
  Ref<TestClassA> a(V_NEW<TestClassA>(0));
  Ref<TestClassA> b(V_NEW<TestAllocator, TestClassA>(&g_alloc, 0));
  ASSERT_NE(a, b);
  ASSERT_NE(a.get(), b.get());

  a = b;
  ASSERT_EQ(a, b);
  ASSERT_EQ(a.get(), b.get());

  a.reset();
  ASSERT_EQ(a, nullptr);
  ASSERT_EQ(a.get(), nullptr);

  a = std::move(b);
  ASSERT_EQ(b.get(), nullptr);
  ASSERT_EQ(b, nullptr);

  ASSERT_NE(a, nullptr);
  ASSERT_NE(a.get(), nullptr);
  ASSERT_EQ(a->refCnt()->refCount(), 1);
}

TEST(Memory, multithread)
{
  using std::default_random_engine;
  using std::uniform_int_distribution;

  auto               threadNum = 10;
  ThreadPool         pool(threadNum);
  Ref<TestClassA>    ptr(V_NEW<TestClassA>(5));
  std::atomic_size_t c = 1;

  for (int i = 0; i < threadNum; i++)
  {
    pool.appendTask(
      [&]()
      {
        default_random_engine e;
        e.seed(123453);
        uniform_int_distribution<int> u;
        std::vector<Ref<TestClassA>>  vec;
        int                           count = 100;
        Timer                         t;
        while (count--)
        {
          if (u(e) > 0)
          {
            vec.push_back(Ref<TestClassA>(ptr));
            c++;
          }
          else
          {
            if (vec.size() != 0)
            {
              vec.back().reset();
              vec.pop_back();
              c--;
            }
            else
            {
              vec.push_back(Ref<TestClassA>(ptr));
              c++;
            }
          }
          std::this_thread::sleep_for(0.1s);
        }
        c -= vec.size();
      });
  }
  pool.wait();
  const auto refcnt = ptr->refCnt()->refCount();
  EXPECT_EQ(refcnt, 1);
  EXPECT_EQ(refcnt, c.load());
  EXPECT_EQ(c.load(), 1);
}
