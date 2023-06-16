#include "Editor/Editor.hpp"

#include "Model/RawJsonDocument.hpp"
#include "Model/Saver/DirSaver.hpp"
#include "Model/Saver/ZipSaver.hpp"

#include <gtest/gtest.h>

#include <filesystem>

using namespace VGG;
namespace fs = std::filesystem;

constexpr auto artboard_file_name = "artboard.json";
constexpr auto event_listeners_file_name = "event_listeners.json";
constexpr auto layout_file_name = "layout.json";

constexpr auto model_src_dir_path = "testDataDir/vgg-work/";

class EditorSaveTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<VggWork> m_model;

  void SetUp() override
  {
    auto fn = [](const json& design_json)
    {
      auto raw_json_doc = new RawJsonDocument();
      raw_json_doc->setContent(design_json);
      return JsonDocumentPtr(raw_json_doc);
    };
    m_model.reset(new VggWork(fn));
  }

  void TearDown() override
  {
  }
};

TEST_F(EditorSaveTestSuite, dir_to_dir)
{
  // Given
  std::string path{ model_src_dir_path };
  m_model->load(path);

  std::string dst_dir{ "tmp/" };
  auto saver{ std::make_shared<Model::DirSaver>(dst_dir) };
  Editor sut{ m_model, saver };

  // When
  sut.save();

  // Then
  std::filesystem::path dir{ dst_dir };
  auto ret = fs::is_regular_file(dir / artboard_file_name);
  EXPECT_TRUE(ret);

  ret = fs::is_regular_file(dir / event_listeners_file_name);
  EXPECT_TRUE(ret);
}

TEST_F(EditorSaveTestSuite, dir_to_zip)
{
  // Given
  std::string path{ model_src_dir_path };
  m_model->load(path);

  std::string dst_file{ "tmp/1.zip" };
  auto saver{ std::make_shared<Model::ZipSaver>(dst_file) };
  Editor sut{ m_model, saver };

  // When
  sut.save();

  // Then
  auto ret = fs::is_regular_file(dst_file);
  EXPECT_TRUE(ret);
}