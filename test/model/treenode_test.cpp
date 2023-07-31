#include "Basic/Node.h"
#include "nlohmann/json.hpp"
#include <deque>
#include <gtest/gtest.h>
#include <queue>

using namespace nlohmann::literals;
using namespace nlohmann;
using namespace VGG;

template<typename F>
void PreorderVisit(const NodePtr& root, F&& f)
{
  if (root)
  {
    f(root);
    for (const auto& p : root->m_firstChild)
    {

      PreorderVisit(p, std::forward<F>(f));
    }
  }
}

template<typename F>
void PostorderVisit(const NodePtr& root, F&& f)
{
  if (root)
  {
    for (const auto& p : root->m_firstChild)
    {
      PostorderVisit(p, std::forward<F>(f));
    }
    f(root);
  }
}

class TreeNodeTestSuit : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }
  void TearDown() override
  {
  }

  static VGG::NodePtr root;

  static void check()
  {
    std::deque<std::string> cases = { "1", "2",  "6",  "7",  "12", "8",  "3", "4",
                                      "9", "10", "13", "14", "11", "15", "5" };
    preOrder(root, cases);
    cases = { "6", "12", "7", "8", "2", "3", "9", "13", "14", "10", "15", "11", "4", "5", "1" };
    postOrder(root, cases);
  }

  static void print()
  {
    PreorderVisit(root, [](const auto& p) { std::cout << p->m_name << " "; });
    std::cout << "\n";
  }

  static void SetUpTestCase()
  {
    root = VGG::Node::createNode("1");
  }
  static void TearDownTestCase()
  {
  }

  static VGG::NodePtr initTreeFromJson(const json& j)
  {
    // each json value is an array, whose element is either
    // a string(leaf node) or an object(non-leaf node)
    if (j.is_array())
    {
      VGG::NodePtr tree = VGG::Node::createNode(j[0].get<std::string>());
      return tree;
    }
    else if (j.is_object())
    {
      return fromObject(j);
    }
    return nullptr;
  }

  static VGG::NodePtr fromObject(const nlohmann::json& j)
  {
    EXPECT_EQ(j.size(), 1);
    VGG::NodePtr tree;
    for (const auto& [k, v] : j.items())
    {
      tree = VGG::Node::createNode(k);
      fromArray(v, tree);
    }
    return tree;
  }
  static VGG::NodePtr fromArray(const nlohmann::json& j, VGG::NodePtr tree)
  {
    for (const auto& e : j)
    {
      if (e.is_object())
      {
        tree->pushChildBack(fromObject(e));
      }
      else if (e.is_string())
      {
        tree->pushChildBack(VGG::Node::createNode(e.get<std::string>()));
      }
    }
    return tree;
  }

  static void postOrder(NodePtr root, std::deque<std::string>& cases)
  {
    PostorderVisit(root,
                   [&cases](const auto& p)
                   {
                     EXPECT_FALSE(cases.empty());
                     EXPECT_EQ(p->m_name, cases.front());
                     cases.pop_front();
                   });
  }

  static void preOrder(NodePtr root, std::deque<std::string>& cases)
  {
    PreorderVisit(root,
                  [&cases](const auto& p)
                  {
                    EXPECT_FALSE(cases.empty());
                    EXPECT_EQ(p->m_name, cases.front());
                    cases.pop_front();
                  });
  }
};

VGG::NodePtr TreeNodeTestSuit::root = nullptr;

TEST_F(TreeNodeTestSuit, Create)
{
  root = Node::createNode("1");
}

TEST_F(TreeNodeTestSuit, InitTreeFromJson)
{
  // clang-format off
//                               /-6
//                              |
//                     /2-------|-7------- /-12
//                    |         |
//                    |          \-8
//                    |
//                    |--3
//                    |
//                    |          /-9
// -1------- /--------|         |
//                    |         |          /-13
//                    |-4-------|-10------|
//                    |         |          \-14
//                    |         |
//                    |          \11------ /-15
//                    |
//                     \-5
  // clang-format on
  const char* str = R"json(
		{"1":
			[
				{"2":
					["6",
					{"7":["12"]},
						"8"]
				},
				"3",
				{"4":
					[
						"9",
						{"10": ["13", "14"]},
						{"11":["15"]}
					]
				},
				"5"
			]
		}
		)json";

  auto j = json::parse(str);
  initTreeFromJson(j);
}

TEST_F(TreeNodeTestSuit, Print)
{
  print();
}

TEST_F(TreeNodeTestSuit, TraverseCheck)
{
  check();
}

TEST_F(TreeNodeTestSuit, RemoveChildAndReattach)
{
  auto removed = root->removeChild("4");

  std::deque<std::string> cases = { "4", "9", "10", "13", "14", "11", "15" };
  preOrder(removed, cases);

  cases = { "1", "2", "6", "7", "12", "8", "3", "5" };
  preOrder(root, cases);
  root->pushChildAt("5", removed);

  cases = { "1", "2", "6", "7", "12", "8", "3", "4", "9", "10", "13", "14", "11", "15", "5" };
  preOrder(root, cases);
}

TEST_F(TreeNodeTestSuit, RemoveSiblingAndReattach)
{
  auto removed = root->removeChild("4");

  std::deque<std::string> cases = { "4", "9", "10", "13", "14", "11", "15" };
  preOrder(removed, cases);

  cases = { "1", "2", "6", "7", "12", "8", "3", "5" };
  preOrder(root, cases);
  root->findChild("3")->pushSiblingAt("5", removed);

  cases = { "1", "2", "6", "7", "12", "8", "3", "4", "9", "10", "13", "14", "11", "15", "5" };
  preOrder(root, cases);
}

TEST_F(TreeNodeTestSuit, FindChildNode)
{
  auto c = root->findChild("1");
  EXPECT_FALSE(c);

  c = root->findChild("14");
  EXPECT_FALSE(c);
  c = root->findChildRecursive("14");
  EXPECT_TRUE(c != nullptr);
  EXPECT_EQ("14", c->m_name);
}

TEST_F(TreeNodeTestSuit, FindSiblingNode)
{
  auto c = root->findChild("10");
  EXPECT_TRUE(c);
  EXPECT_FALSE(c->findNextSblingFromCurrent("9"));
  EXPECT_TRUE(c->findNextSblingFromCurrent("10"));
  EXPECT_TRUE(c->findNextSblingFromCurrent("11"));
  // prev find
  EXPECT_TRUE(c->findPrevSiblingFromCurrent("9"));
  EXPECT_FALSE(c->findNextSblingFromCurrent("10"));
  EXPECT_FALSE(c->findNextSblingFromCurrent("11"));
}

TEST_F(TreeNodeTestSuit, SeekRoot)
{
  auto c = root->findChildRecursive("10");
  EXPECT_TRUE(c != nullptr);
  EXPECT_EQ("10", c->m_name);
  auto r = c->root();
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ("1", r->m_name);
  // removed
  auto removed = root->removeChild("4");
  r = c->root();
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ("4", r->m_name);

  root->pushChildAt("5", removed);
  check();
  r = c->root();
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ("1", r->m_name);
}

TEST_F(TreeNodeTestSuit, AccessByIterator)
{
  auto c = root->findChildRecursive("10");
  EXPECT_TRUE(c);
  EXPECT_EQ(c->m_name, "10");
  auto it = *(c->iter);
  EXPECT_EQ(it->m_name, "10");
}
