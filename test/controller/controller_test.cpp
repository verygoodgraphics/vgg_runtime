#include "Controller.hpp"
#include "MainComposer.hpp"
#include "MockJsonDocumentObserver.hpp"
#include "Model/VggWork.hpp"
#include "VggDepContainer.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <mutex>
#include <condition_variable>

using ::testing::_;
using ::testing::Return;

constexpr auto design_doc_schema_file = "./asset/vgg-format.json";

class ControllerTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Controller> m_sut;
  MainComposer composer;

  void SetUp() override
  {
    composer.setup("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
  }
  void TearDown() override
  {
    composer.teardown();
  }
};

TEST_F(ControllerTestSuite, Smoke)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  m_sut.reset(new Controller);

  // When
  auto ret = m_sut->start(file_path);

  // Then
  EXPECT_TRUE(ret);
  // Then
  auto vgg_work = VggDepContainer<std::shared_ptr<VggWork>>::get();
  EXPECT_TRUE(vgg_work);
}

TEST_F(ControllerTestSuite, OnClick_observer)
{
  // Given
  auto mock_observer = new MockJsonDocumentObserver();
  std::mutex m;
  std::condition_variable cv;

  m_sut.reset(new Controller(JsonDocumentObserverPtr(mock_observer)));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path);
  EXPECT_TRUE(ret);

  auto callback = [&](const json::json_pointer&)
  {
    std::unique_lock lk(m);
    lk.unlock();
    cv.notify_one();
  };

  auto vgg_work = VggDepContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  EXPECT_CALL(*mock_observer, didDelete(_)).WillOnce(callback);

  // When
  m_sut->onClick("/artboard/layers/0/childObjects");

  // Then
  // wait node thread change model // note: thread safe
  {
    std::unique_lock lk(m);
    cv.wait(lk);
  }
  auto new_design_doc_json = vgg_work->designDoc()->content();
  EXPECT_FALSE(design_doc_json == new_design_doc_json);
}

TEST_F(ControllerTestSuite, Validator_reject_deletion)
{
  // Given
  auto mock_observer = new MockJsonDocumentObserver();

  m_sut.reset(new Controller(JsonDocumentObserverPtr(mock_observer)));
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->start(file_path, design_doc_schema_file);
  EXPECT_TRUE(ret);

  auto vgg_work = VggDepContainer<std::shared_ptr<VggWork>>::get();
  auto design_doc_json = vgg_work->designDoc()->content();

  // expect call
  EXPECT_CALL(*mock_observer, didDelete(_)).Times(0);

  // When
  m_sut->onClick("/artboard/layers/0/childObjects");

  // Then
  using ::testing::Mock;
  Mock::VerifyAndClearExpectations(mock_observer);
}