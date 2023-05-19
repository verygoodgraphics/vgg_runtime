#include "Controller/Controller.hpp"

#include "Model/VggWork.hpp"
#include "PlatformAdapter/Native/Composer/MainComposer.hpp"
#include "Utils/DIContainer.hpp"

#include "rxcpp/rx.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <mutex>
#include <condition_variable>

using namespace VGG;

constexpr auto design_doc_schema_file = "./asset/vgg-format.json";

class ControllerTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Controller> m_sut;
  MainComposer composer;
  std::shared_ptr<RunLoop> m_run_loop = std::make_shared<RunLoop>();
  bool m_exit_loop = false;

  void SetUp() override
  {
  }

  void TearDown() override
  {
    composer.teardown();
  }

  void setup_sdk_with_local_dic()
  {
    composer.setup("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
  }
  void setup_sdk_with_remote_dic()
  {
    composer.setup("./asset/vgg-sdk.esm.mjs");
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
};

TEST_F(ControllerTestSuite, Smoke)
{
  // Given
  setup_sdk_with_local_dic();
  std::string file_path = "testDataDir/vgg-work.zip";
  m_sut.reset(new Controller(m_run_loop));

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
  auto fake_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });

  m_sut.reset(new Controller(m_run_loop, fake_design_doc_observer));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  m_sut->onClick("/artboard/layers/0/childObjects");
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
  auto fake_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  m_sut.reset(new Controller(m_run_loop, fake_design_doc_observer));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  m_sut->onClick("/artboard/layers/0/childObjects");
  loop_times(100);

  // Then
  EXPECT_TRUE(type != ModelEventType::Delete);
}

TEST_F(ControllerTestSuite, DidUpdate)
{
  // Given
  setup_sdk_with_local_dic();

  auto type = ModelEventType::Invalid;
  auto fake_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  m_sut.reset(new Controller(m_run_loop, fake_design_doc_observer));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  m_sut->onClick("/artboard/layers");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Update);
}

TEST_F(ControllerTestSuite, DidDelete)
{
  // Given
  setup_sdk_with_local_dic();

  auto type = ModelEventType::Invalid;
  auto fake_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  m_sut.reset(new Controller(m_run_loop, fake_design_doc_observer));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  m_sut->onClick("/artboard/layers/0");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Delete);
}

TEST_F(ControllerTestSuite, DidAdd_no_validator)
{
  // Given
  setup_sdk_with_local_dic();

  auto type = ModelEventType::Invalid;
  auto fake_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  m_sut.reset(new Controller(m_run_loop, fake_design_doc_observer));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  m_sut->onClick("/fake/add");
  loop_until_exit();

  //

  // Then
  EXPECT_TRUE(type == ModelEventType::Add);
}

TEST_F(ControllerTestSuite, DidAdd_color)
{
  // Given
  setup_sdk_with_remote_dic();

  auto type = ModelEventType::Invalid;
  auto fake_design_doc_observer = rxcpp::make_observer_dynamic<ModelEventPtr>(
    [&](ModelEventPtr evt)
    {
      type = evt->type;
      m_exit_loop = true;
    });
  m_sut.reset(new Controller(m_run_loop, fake_design_doc_observer));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VGG::DIContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // When
  m_sut->onClick("/fake/add_color");
  loop_until_exit();

  // Then
  EXPECT_TRUE(type == ModelEventType::Add);
}