#include "VggSdk.hpp"
#include "VggDepContainer.hpp"
#include "MockDocumentModel.hpp"
#include "MockJsonSchemaValidator.hpp"

#include <gtest/gtest.h>

#include <memory>

class VggSdkTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<VggSdk> m_sut_ptr;
  MockDocumentModel* m_mock_document_model = nullptr;

  void SetUp() override
  {
    m_sut_ptr.reset(new VggSdk);
  }

  void TearDown() override
  {
  }
};

TEST_F(VggSdkTestSuite, Smoke)
{
  // Given
  std::string original_string("{}");

  auto mock_json_validator = std::make_shared<MockJsonSchemaValidator>();
  json mock_doc_json(original_string);
  m_mock_document_model = new MockDocumentModel(mock_json_validator, mock_doc_json);
  VggDepContainer<std::shared_ptr<DocumentModel>>::get().reset(m_mock_document_model);

  // When
  auto result = m_sut_ptr->documentJson();

  // Then
  EXPECT_EQ(result, original_string);
}