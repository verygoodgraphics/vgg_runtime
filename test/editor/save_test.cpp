#include "Usecase/SaveModel.hpp"

#include "Domain/RawJsonDocument.hpp"
#include "Domain/Saver/DirSaver.hpp"
#include "Domain/Saver/ZipSaver.hpp"

#include <gtest/gtest.h>

#include <filesystem>

using namespace VGG;
using namespace VGG::Model;
namespace fs = std::filesystem;

constexpr auto model_src_dir_path = "testDataDir/vgg-daruma/";
constexpr auto model_src_zip_path = "testDataDir/vgg-daruma.zip";

class EditorSaveTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Daruma> m_model;

  void SetUp() override
  {
    auto fn = [](const json& design_json)
    {
      auto raw_json_doc = new RawJsonDocument();
      raw_json_doc->setContent(design_json);
      return JsonDocumentPtr(raw_json_doc);
    };
    m_model.reset(new Daruma(fn));
  }

  void TearDown() override
  {
  }

  void load_from_dir()
  {
    // Given
    std::string path{ model_src_dir_path };
    m_model->load(path);
  }

  void load_from_zip()
  {
    // Given
    std::string path{ model_src_zip_path };
    m_model->load(path);
  }

  void save_to_dir()
  {
    std::string dstDir{ "tmp/" };
    auto saver{ std::make_shared<Model::DirSaver>(dstDir) };
    Editor sut{ m_model, saver };

    // When
    sut.save();

    // Then
    std::filesystem::path dir{ dstDir };
    auto ret = fs::is_regular_file(dir / design_file_name);
    EXPECT_TRUE(ret);

    ret = fs::is_regular_file(dir / event_listeners_file_name);
    EXPECT_TRUE(ret);
  }

  void save_to_zip()
  {
    std::string dst_file{ "tmp/1.zip" };
    auto saver{ std::make_shared<Model::ZipSaver>(dst_file) };
    Editor sut{ m_model, saver };

    // When
    sut.save();

    // Then
    auto ret = fs::is_regular_file(dst_file);
    EXPECT_TRUE(ret);
  }
};

TEST_F(EditorSaveTestSuite, dir_to_dir)
{
  load_from_dir();
  save_to_dir();
}

TEST_F(EditorSaveTestSuite, dir_to_zip)
{
  load_from_dir();
  save_to_zip();
}

TEST_F(EditorSaveTestSuite, zip_to_dir)
{
  load_from_zip();
  save_to_dir();
}

TEST_F(EditorSaveTestSuite, zip_to_zip)
{
  load_from_zip();
  save_to_zip();
}