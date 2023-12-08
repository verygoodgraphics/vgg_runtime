#include "Application/Controller.hpp"

#include "Application/RunLoop.hpp"
#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Adapter/NativeComposer.hpp"
#include "mocks/MockPresenter.hpp"

#include <rxcpp/rx.hpp>

#include "test_config.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <mutex>
#include <condition_variable>

using namespace VGG;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

#define SKIP_LOCAL_TEST                                                                            \
  GTEST_SKIP() << "Skipping local './xxx.sdk.mjd' test, unsupported when evaluating data uri ";

class ControllerTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Controller>     m_sut;
  std::shared_ptr<NativeComposer> m_nativeComposer;
  std::shared_ptr<RunLoop>        m_runLoop = RunLoop::sharedInstance();
  std::shared_ptr<MockPresenter>  m_mockPresenter = std::make_shared<MockPresenter>();

  rxcpp::subjects::subject<VGG::UIEventPtr> m_fakeViewSubject;
  bool                                      m_exitLoop = false;

  void SetUp() override
  {
  }

  void TearDown() override
  {
    if (m_nativeComposer)
    {
      m_nativeComposer->teardown();
    }
  }

  auto setupSdkWithLocalDic(bool catchJsException = false)
  {
    m_nativeComposer.reset(new NativeComposer(catchJsException));
    return m_nativeComposer->setup();
  }

  auto setupSdkWithRemoteDic(bool catchJsException = false)
  {
    m_nativeComposer.reset(new NativeComposer(catchJsException));
    return m_nativeComposer->setup();
  }

  auto setupUsingS5Sdk(bool catchJsException = false)
  {
    m_nativeComposer.reset(new NativeComposer(catchJsException));
    return m_nativeComposer->setup();
  }

  void setupSut(std::shared_ptr<VggExec> jsEngine)
  {
    EXPECT_CALL(*m_mockPresenter, getObservable())
      .WillOnce(Return(m_fakeViewSubject.get_observable()));
    m_sut.reset(new Controller(m_runLoop, jsEngine, m_mockPresenter, {}));
  }

  void loopUntilExit()
  {
    while (!m_exitLoop)
    {
      m_runLoop->dispatch();
    }
  }

  void loopTimes(int times)
  {
    while (times--)
    {
      m_runLoop->dispatch();
    }
  }

  void mockClick(const std::string& path, int button = 0)
  {
    m_fakeViewSubject.get_subscriber().on_next(
      UIEventPtr{ new MouseEvent{ path, EUIEventType::CLICK, button } });
  }

  void mockKeydown(const std::string& path)
  {
    m_fakeViewSubject.get_subscriber().on_next(
      UIEventPtr{ new KeyboardEvent{ path, EUIEventType::KEYDOWN, 'a' } });
  }

  void mockTouch(const std::string& path)
  {
    m_fakeViewSubject.get_subscriber().on_next(
      UIEventPtr{ new TouchEvent{ path, EUIEventType::TOUCHSTART } });
  }

  auto getDaruma()
  {
    return DarumaContainer().get();
  }
};

TEST_F(ControllerTestSuite, Smoke)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>([&](ModelEventPtr evt) {});
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);

  // When
  auto ret = m_sut->start(filePath);

  // Then
  EXPECT_TRUE(ret);
  // Then
  auto vggWork = getDaruma();
  EXPECT_TRUE(vggWork);
}

TEST_F(ControllerTestSuite, OnClick_observer)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });

  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/artboard/layers/0/childObjects");
  loopUntilExit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Delete);

  auto newDesignDocJson = vggWork->designDoc()->content();
  EXPECT_FALSE(designDocJson == newDesignDocJson);
}

TEST_F(ControllerTestSuite, Validator_reject_deletion)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/artboard/layers/0/childObjects");
  loopTimes(100);

  // Then
  EXPECT_TRUE(type != ModelEventType::Delete);
}

TEST_F(ControllerTestSuite, DidUpdate)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto udpateEventPtr = static_cast<ModelEventUpdate*>(evt.get());
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/artboard/layers");
  loopUntilExit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Update);
}

TEST_F(ControllerTestSuite, DidDelete)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto deleteEventPtr = static_cast<ModelEventDelete*>(evt.get());
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/artboard/layers/0");
  loopUntilExit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Delete);
}

TEST_F(ControllerTestSuite, DidAdd_no_validator)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto addEventPtr = static_cast<ModelEventAdd*>(evt.get());
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/fake/add");
  loopUntilExit();

  //

  // Then
  EXPECT_TRUE(type == ModelEventType::Add);
}

TEST_F(ControllerTestSuite, DidAdd_color)
{
  SKIP_S3_DEPENDENT_TEST

  // Given
  auto jsEngine = setupSdkWithRemoteDic();

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/fake/add_color");
  loopUntilExit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Add);
}

TEST_F(ControllerTestSuite, add_event_listener)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  // When
  mockClick("/fake/add_event_listener");

  //
  loopUntilExit(); // add 1st listener
  m_exitLoop = false;
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  loopUntilExit(); // add 2nd listener

  // Then
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

TEST_F(ControllerTestSuite, eval_added_event_listener)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  mockClick("/fake/add_event_listener");
  loopUntilExit(); // loop until added
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  // When
  mockClick("/js/fake/run_added_listener");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // Then
  // added listener evaluated
}

TEST_F(ControllerTestSuite, remove_event_listener)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto recvEvtCount = 0;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      recvEvtCount++;
      if (recvEvtCount == 2)
      {
        m_exitLoop = true;
      }
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  // When
  mockClick("/fake/remove_event_listener");

  //
  loopUntilExit();

  // Then
  EXPECT_TRUE(type == ModelEventType::ListenerDidRemove);
}

TEST_F(ControllerTestSuite, get_event_listeners)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  // When
  mockClick("/fake/get_event_listeners");

  loopUntilExit(); // loop until added
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // Then
  // js throw error if failed
}

TEST_F(ControllerTestSuite, unhandled_js_error)
{
  // Given
  auto jsEngine = setupSdkWithLocalDic(true); // enable catch exception
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);

  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  // When
  mockClick("/fake/throw_error");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Then
  // js throw error if failed
}

TEST_F(ControllerTestSuite, event_listener_example)
{
  SKIP_S3_DEPENDENT_TEST

  // Given
  auto jsEngine = setupSdkWithRemoteDic();

  auto type = ModelEventType::Invalid;
  auto fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });
  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->start(filePath, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vggWork = getDaruma();
  auto designDocJson = vggWork->designDoc()->content();

  // When
  mockClick("/fake/event_listener_example");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Then
  // js throw error if failed
}

TEST_F(ControllerTestSuite, handle_events)
{
  SKIP_S3_DEPENDENT_TEST

  // Given
  auto jsEngine = setupUsingS5Sdk();

  int            times = 0;
  auto           type = ModelEventType::Invalid;
  constexpr auto EXPECT_TIMES = 4;
  auto           fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      times++;

      type = evt->type;
      m_exitLoop = times == EXPECT_TIMES;
    });

  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  EXPECT_CALL(*m_mockPresenter, setModel(_));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma-2";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  // When
  mockClick("/fake/handle_event");
  mockClick("/fake/handle_event", 2);
  mockKeydown("/fake/handle_event");
  mockTouch("/fake/handle_event");

  // loopTimes
  //  10000: error
  // 100000: success
  // loopTimes(100000);
  loopUntilExit();

  // Then
}

TEST_F(ControllerTestSuite, handle_event_keyboard)
{
  SKIP_S3_DEPENDENT_TEST

  // Given
  auto jsEngine = setupUsingS5Sdk();

  constexpr auto EXPECT_TIMES = 1;
  auto           type = ModelEventType::Invalid;
  auto           fakeModelObserver = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exitLoop = true;
    });

  EXPECT_CALL(*m_mockPresenter, getModelObserver()).WillOnce(ReturnRef(fakeModelObserver));
  EXPECT_CALL(*m_mockPresenter, setModel(_));
  setupSut(jsEngine);
  std::string filePath = "testDataDir/vgg-daruma-2";
  auto        ret = m_sut->start(filePath);
  EXPECT_TRUE(ret);

  // When
  mockKeydown("/fake/handle_event");

  // loopTimes
  //  10000: error
  // 100000: success
  // loopTimes(100000);
  loopUntilExit();

  // Then
}
