#include "Controller/Controller.hpp"

#include "test_config.hpp"

#include "Model/VggWork.hpp"
#include "PlatformAdapter/Native/Composer/MainComposer.hpp"
#include "Utils/DIContainer.hpp"
#include "mocks/MockPresenter.hpp"

#include "rxcpp/rx.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <mutex>
#include <condition_variable>

using namespace VGG;
using ::testing::Return;
using ::testing::ReturnRef;

constexpr auto design_doc_schema_file = "./asset/vgg-format.json";

class ControllerTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Controller> m_sut;
  MainComposer m_main_composer{ false }; // false: enable js throw error to abort subprocess
  std::shared_ptr<RunLoop> m_run_loop = std::make_shared<RunLoop>();
  MockPresenter m_mock_presenter;
  rxcpp::subjects::subject<VGG::UIEventPtr> m_fake_view_subject;
  bool m_exit_loop = false;

  void SetUp() override
  {
  }

  void TearDown() override
  {
    m_main_composer.teardown();
  }

  void setup_sdk_with_local_dic()
  {
    m_main_composer.setup("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
  }
  void setup_sdk_with_remote_dic()
  {
    m_main_composer.setup("./asset/vgg-sdk.esm.mjs");
  }

  void setup_sut()
  {
    EXPECT_CALL(m_mock_presenter, getObservable())
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

  void mock_click(const std::string& path)
  {
    m_fake_view_subject.get_subscriber().on_next(
      UIEventPtr{ new UIEvent{ path, UIEventType::click, MouseEvent{} } });
  }
};

TEST_F(ControllerTestSuite, Smoke)
{
  // Given
  setup_sdk_with_local_dic();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto fake_design_doc_observer =
    rxcpp::make_observer_dynamic<ModelEventPtr>([&](ModelEventPtr evt) {});
  EXPECT_CALL(m_mock_presenter, getDesignDocObserver())
    .WillOnce(ReturnRef(fake_design_doc_observer));
  setup_sut();

  // When
  auto ret = m_sut->start(file_path);

  // Then
  EXPECT_TRUE(ret);
  // Then
  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  EXPECT_TRUE(vgg_work);
}

TEST_F(ControllerTestSuite, OnClick_observer)
{
  // Given
  setup_sdk_with_local_dic();

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });

  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto udpate_event_ptr = static_cast<ModelEventUpdate*>(evt.get());
      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto delete_event_ptr = static_cast<ModelEventDelete*>(evt.get());
      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      auto add_event_ptr = static_cast<ModelEventAdd*>(evt.get());
      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
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
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(ReturnRef(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(Return(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(Return(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
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
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(Return(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
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

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(Return(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
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
  m_main_composer = MainComposer(true); // enable catch exception
  setup_sdk_with_local_dic();

  auto type = ModelEventType::Invalid;
  auto fake_model_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;

      m_exit_loop = true;
    });
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(Return(fake_model_observer));
  setup_sut();

  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  // When
  mock_click("/fake/throw_error");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // std::this_thread::sleep_for(std::chrono::milliseconds(10000));

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
  EXPECT_CALL(m_mock_presenter, getModelObserver()).WillOnce(Return(fake_model_observer));
  setup_sut();
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  mock_click("/fake/event_listener_example");

  // wait for evaluating
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Then
  // js throw error if failed
}