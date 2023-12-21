#include "Layer/Core/VBound.hpp"
#include "Layer/Core/VNode.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Layer/Memory/VNew.hpp"
#include <nlohmann/json.hpp>
#include <deque>
#include <gtest/gtest.h>
using namespace VGG::layer;

class TestObserveNode : public VNode
{
public:
  Ref<TestObserveNode> child;
  std::vector<int>     vec;
  std::string          name;
  TestObserveNode(VRefCnt* cnt, std::string name, Ref<TestObserveNode> child = nullptr)
    : VNode(cnt)
    , name(std::move(name))
    , vec{ 1, 2, 3, 4 }
    , child(std::move(child))
  {
    if (this->child)
    {
      this->observe(this->child);
    }
  }

protected:
  VGG::Bound onRevalidate() override
  {
    if (child)
      child->revalidate();
    std::cerr << "onRevalidate: " << name << std::endl;
    for (auto i : vec)
      std::cerr << i << " ";
    std::cerr << std::endl;
    return {};
  }
};

TEST(ObserveTest, SimpleCase)
{
  Ref<TestObserveNode> child2 = Ref<TestObserveNode>(V_NEW<TestObserveNode>("child2"));
  Ref<TestObserveNode> child1 =
    Ref<TestObserveNode>(V_NEW<TestObserveNode>("child1", std::move(child2)));
  Ref<TestObserveNode> root =
    Ref<TestObserveNode>(V_NEW<TestObserveNode>("root", std::move(child1)));
  root->child->child->invalidate();
  root->revalidate();
}
