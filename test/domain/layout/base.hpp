#include "Domain/Layout/Layout.hpp"
#include "UseCase/StartRunning.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

using namespace VGG;

class BaseVggLayoutTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Layout::Layout> m_sut;

  void SetUp() override
  {
  }

  void TearDown() override
  {
  }

  auto setupModel(const char* fileDir)
  {
    std::shared_ptr<Daruma> daruma{
      new Daruma(Helper::RawJsonDocumentBuilder, Helper::RawJsonDocumentBuilder)
    };
    daruma->load(fileDir);
    return daruma;
  }

  void setup(const char* fileDir)
  {
    auto daruma = setupModel(fileDir);
    m_sut.reset(new Layout::Layout{ daruma->designDoc(), daruma->layoutDoc() });
  }

  void setupWithExpanding(const char* fileDir)
  {
    auto daruma = setupModel(fileDir);

    StartRunning startRunning{ daruma };
    m_sut = startRunning.layout();
  }

  auto page(std::size_t index)
  {
    auto tree = m_sut->layoutTree();
    return tree->children()[index];
  }

  auto firstPage()
  {
    return page(0);
  }

  auto childFrame(int index)
  {
    auto frame = firstPage()->children()[index]->frame();
    return frame;
  }

  auto grandsonFrame(int index, int index2)
  {
    auto frame = firstPage()->children()[index]->children()[index2]->frame();
    return frame;
  }

  auto descendantFrame(std::vector<int> indexs, std::size_t pageIndex = 0)
  {
    auto node = page(pageIndex);
    for (auto index : indexs)
    {
      node = node->children()[index];
    }

    auto frame = node->frame();
    return frame;
  }
};