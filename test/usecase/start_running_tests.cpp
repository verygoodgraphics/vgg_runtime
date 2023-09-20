#include "UseCase/StartRunning.hpp"

#include "domain/model/daruma_helper.hpp"
#include "test_config.hpp"

#include <gtest/gtest.h>

using namespace VGG;
using namespace VGG::Layout;

class StartRunningTestSuite : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }

  void TearDown() override
  {
  }
};

TEST_F(StartRunningTestSuite, Smoke)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/0_space_between/";
  daruma->load(filePath);

  // When
  StartRunning sut{ daruma };

  // Then
}
