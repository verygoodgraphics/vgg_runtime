#include "Application/Controller.hpp"

#include "test_config.hpp"

#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Adapter/NativeComposer.hpp"
#include "DIContainer.hpp"
#include "mocks/MockPresenter.hpp"

#include "rxcpp/rx.hpp"

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
  std::shared_ptr<Controller> m_sut;
  std::shared_ptr<NativeComposer> m_native_composer;
  std::shared_ptr<RunLoop> m_run_loop = std::make_shared<RunLoop>();
  std::shared_ptr<MockPresenter> m_mock_presenter = std::make_shared<MockPresenter>();

  rxcpp::subjects::subject<VGG::UIEventPtr> m_fake_view_subject;
  bool m_exit_loop = false;

  void SetUp() override
  {
  }

  void TearDown() override
  {
    if (m_native_composer)
    {
      m_native_composer->teardown();
    }
  }

  void setup_sdk_with_local_dic(bool catchJsException = false)
  {
    m_native_composer.reset(
      new NativeComposer("./testDataDir/fake-sdk/vgg-sdk.esm.mjs", catchJsException));
    m_native_composer->setup();
  }

  void setup_sdk_with_remote_dic(bool catchJsException = false)
  {
    m_native_composer.reset(new NativeComposer("./asset/vgg-sdk.esm.mjs", catchJsException));
    m_native_composer->setup();
  }

  void setup_using_s5_sdk(bool catchJsException = false)
  {
    m_native_composer.reset(
      new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js", catchJsException));
    m_native_composer->setup();
  }

  void setup_sut()
  {
    EXPECT_CALL(*m_mock_presenter, getObservable())
      .WillOnce(Return(m_fake_view_subject.get_observable()));
    m_sut.reset(new Controller(m_run_loop, m_mock_presenter));
  }

  void loop_until_exit()
  {
    while (!m_exit_loop)
    {
      m_run_loop->dispatch();
    }
  }

  void loop_times(int times)
  {
    while (times--)
    {
      m_run_loop->dispatch();
    }
  }

  void mock_click(const std::string& path, int button = 0)
  {
    m_fake_view_subject.get_subscriber().on_next(
      UIEventPtr{ new MouseEvent{ path, UIEventType::click, button } });
  }

  void mock_keydown(const std::string& path)
  {
    m_fake_view_subject.get_subscriber().on_next(
      UIEventPtr{ new KeyboardEvent{ path, UIEventType::keydown, 'a' } });
  }

  void mock_touch(const std::string& path)
  {
    m_fake_view_subject.get_subscriber().on_next(
      UIEventPtr{ new TouchEvent{ path, UIEventType::touchstart } });
  }

  auto get_daruma()
  {
    return DarumaContainer().get();
  }
};

TEST_F(ControllerTestSuite, Smoke)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>([&](ModelEventPtr evt) {});
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();

  // When
  auto ret = m_sut->start(file_path);

  // Then
  EXPECT_TRUE(ret);
  // Then
  auto vgg_work = get_daruma();
  EXPECT_TRUE(vgg_work);
}

TEST_F(ControllerTestSuite, OnClick_observer)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });

  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/artboard/layers/0/childObjects");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Delete);

  auto new_design_doc_json = vgg_work->designDoc()->content();
  EXPECT_FALSE(design_doc_json == new_design_doc_json);
}

TEST_F(ControllerTestSuite, Validator_reject_deletion)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/artboard/layers/0/childObjects");
  loop_times(100);

  // Then
  EXPECT_TRUE(type != ModelEventType::Delete);
}

TEST_F(ControllerTestSuite, DidUpdate)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto udpate_event_ptr = static_cast<ModelEventUpdate*>(evt.get());
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/artboard/layers");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Update);
}

TEST_F(ControllerTestSuite, DidDelete)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto delete_event_ptr = static_cast<ModelEventDelete*>(evt.get());
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/artboard/layers/0");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Delete);
}

TEST_F(ControllerTestSuite, DidAdd_no_validator)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto add_event_ptr = static_cast<ModelEventAdd*>(evt.get());
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/fake/add");
  loop_until_exit();

  //

  // Then
  EXPECT_TRUE(type == ModelEventType::Add);
}

TEST_F(ControllerTestSuite, DidAdd_color)
{
  SKIP_S3_DEPENDENT_TEST

  // Given
  setup_sdk_with_remote_dic();

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/fake/add_color");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Add);
}

TEST_F(ControllerTestSuite, add_event_listener)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_click("/fake/add_event_listener");

  //
  loop_until_exit(); // add 1st listener
  m_exit_loop = false;
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  loop_until_exit(); // add 2nd listener

  // Then
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

TEST_F(ControllerTestSuite, eval_added_event_listener)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  mock_click("/fake/add_event_listener");
  loop_until_exit(); // loop until added
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  // When
  mock_click("/js/fake/run_added_listener");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // Then
  // added listener evaluated
}

TEST_F(ControllerTestSuite, remove_event_listener)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto recv_evt_count = 0;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      recv_evt_count++;
      if (recv_evt_count == 2)
      {
        m_exit_loop = true;
      }
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_click("/fake/remove_event_listener");

  //
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::ListenerDidRemove);
}

TEST_F(ControllerTestSuite, get_event_listeners)
{
  // Given
  setup_sdk_with_local_dic();
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_click("/fake/get_event_listeners");

  loop_until_exit(); // loop until added
  EXPECT_TRUE(type == ModelEventType::ListenerDidAdd);

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // Then
  // js throw error if failed
}

TEST_F(ControllerTestSuite, unhandled_js_error)
{
  // Given
  setup_sdk_with_local_dic(true); // enable catch exception
  SKIP_LOCAL_TEST

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();

  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_click("/fake/throw_error");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Then
  // js throw error if failed
}

TEST_F(ControllerTestSuite, event_listener_example)
{
  SKIP_S3_DEPENDENT_TEST

  // Given
  setup_sdk_with_remote_dic();

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = get_daruma();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/fake/event_listener_example");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Then
  // js throw error if failed
}

TEST_F(ControllerTestSuite, handle_events)
{
  // Given
  setup_using_s5_sdk();

  int times = 0;
  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      times++;

      type = evt->type;
      m_exit_loop = times == 4;
    });

  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  EXPECT_CALL(*m_mock_presenter, setModel(_));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_click("/fake/handle_event");
  mock_click("/fake/handle_event", 2);
  mock_keydown("/fake/handle_event");
  mock_touch("/fake/handle_event");

  // loop_times
  //  10000: error
  // 100000: success
  // loop_times(100000);
  loop_until_exit();

  // Then
}

TEST_F(ControllerTestSuite, handle_event_keyboard)
{
  // Given
  setup_using_s5_sdk();

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });

  EXPECT_CALL(*m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  EXPECT_CALL(*m_mock_presenter, setModel(_));
  setup_sut();
  std::string file_path = "testDataDir/vgg-daruma.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_keydown("/fake/handle_event");

  // loop_times
  //  10000: error
  // 100000: success
  // loop_times(100000);
  loop_until_exit();

  // Then
}
