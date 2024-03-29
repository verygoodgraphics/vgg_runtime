#include "Layer/Core/VNode.hpp"
#include "Layer/Core/TreeNode.hpp"
#include <nlohmann/json.hpp>
#include <deque>
#include <gtest/gtest.h>
#include <queue>

using namespace nlohmann::literals;
using namespace nlohmann;
using namespace VGG::layer;
using namespace VGG;

// NOLINTBEGIN

template<typename F>
void PreorderVisit(TreeNodePtr& root, F&& f)
{
  if (root)
  {
    f(root);
    for (auto& p : *root)
    {
      PreorderVisit(p, std::forward<F>(f));
    }
  }
}

template<typename F>
void PostorderVisit(TreeNodePtr& root, F&& f)
{
  if (root)
  {
    for (auto& p : *root)
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

  static TreeNodePtr s_root;

  static void check()
  {
    std::deque<std::string> cases = { "1", "2",  "6",  "7",  "12", "8",  "3", "4",
                                      "9", "10", "13", "14", "11", "15", "5" };
    preOrder(s_root, cases);
    cases = { "6", "12", "7", "8", "2", "3", "9", "13", "14", "10", "15", "11", "4", "5", "1" };
    postOrder(s_root, cases);
  }

  static void print()
  {
    PreorderVisit(s_root, [](const auto& p) { std::cout << p->name() << " "; });
    std::cerr << "\n";
  }

  static void SetUpTestCase()
  {
    s_root = makeTreeNodePtr("1");
  }
  static void TearDownTestCase()
  {
  }

  static TreeNodePtr initTreeFromJson(const json& j)
  {
    // each json value is an array, whose element is either
    // a string(leaf node) or an object(non-leaf node)
    if (j.is_array())
    {
      TreeNodePtr tree = makeTreeNodePtr(j[0].get<std::string>());
      return tree;
    }
    else if (j.is_object())
    {
      return fromObject(j);
    }
    return nullptr;
  }

  static TreeNodePtr fromObject(const nlohmann::json& j)
  {
    EXPECT_EQ(j.size(), 1);
    TreeNodePtr tree;
    for (const auto& [k, v] : j.items())
    {
      tree = makeTreeNodePtr(k);
      fromArray(v, tree);
    }
    return tree;
  }
  static TreeNodePtr fromArray(const nlohmann::json& j, TreeNodePtr tree)
  {
    for (const auto& e : j)
    {
      if (e.is_object())
      {
        tree->pushChildBack(fromObject(e));
      }
      else if (e.is_string())
      {
        tree->pushChildBack(makeTreeNodePtr(e.get<std::string>()));
      }
    }
    return tree;
  }

  static void postOrder(TreeNodePtr root, std::deque<std::string>& cases)
  {
    PostorderVisit(
      root,
      [&cases](const auto& p)
      {
        EXPECT_FALSE(cases.empty());
        EXPECT_EQ(p->name(), cases.front());
        cases.pop_front();
      });
  }

  static void preOrder(TreeNodePtr root, std::deque<std::string>& cases)
  {
    PreorderVisit(
      root,
      [&cases](const auto& p)
      {
        EXPECT_FALSE(cases.empty());
        EXPECT_EQ(p->name(), cases.front());
        cases.pop_front();
      });
  }

  static void init()
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
    s_root = initTreeFromJson(j);
  }
};

TreeNodePtr TreeNodeTestSuit::s_root = nullptr;

TEST_F(TreeNodeTestSuit, Create)
{
  s_root = makeTreeNodePtr("1");
}

TEST_F(TreeNodeTestSuit, InitTreeFromJson)
{
  init();
}

TEST_F(TreeNodeTestSuit, Print)
{
  init();
  print();
}

TEST_F(TreeNodeTestSuit, TraverseCheck)
{
  init();
  check();
}

TEST_F(TreeNodeTestSuit, RemoveChildAndReattach)
{
  init();
  auto removed = s_root->removeChild("4");

  std::deque<std::string> cases = { "4", "9", "10", "13", "14", "11", "15" };
  preOrder(removed, cases);

  cases = { "1", "2", "6", "7", "12", "8", "3", "5" };
  preOrder(s_root, cases);
  s_root->pushChildAt("5", removed);

  cases = { "1", "2", "6", "7", "12", "8", "3", "4", "9", "10", "13", "14", "11", "15", "5" };
  preOrder(s_root, cases);
}

TEST_F(TreeNodeTestSuit, RemoveSiblingAndReattach)
{
  init();
  auto removed = s_root->removeChild("4");

  std::deque<std::string> cases = { "4", "9", "10", "13", "14", "11", "15" };
  preOrder(removed, cases);

  cases = { "1", "2", "6", "7", "12", "8", "3", "5" };
  preOrder(s_root, cases);
  s_root->findChild("3")->pushSiblingAt("5", removed);

  cases = { "1", "2", "6", "7", "12", "8", "3", "4", "9", "10", "13", "14", "11", "15", "5" };
  preOrder(s_root, cases);
}

TEST_F(TreeNodeTestSuit, FindChildNode)
{
  init();
  auto c = s_root->findChild("1");
  EXPECT_FALSE(c);

  c = s_root->findChild("14");
  EXPECT_FALSE(c);
  c = s_root->findChildRecursive("14");
  EXPECT_TRUE(c != nullptr);
  EXPECT_EQ("14", c->name());
}

TEST_F(TreeNodeTestSuit, FindSiblingNode)
{
  init();
  auto c = s_root->findChildRecursive("10");
  EXPECT_TRUE(c);
  EXPECT_FALSE(c->findNextSblingFromCurrent("9"));
  EXPECT_TRUE(c->findNextSblingFromCurrent("10"));
  EXPECT_TRUE(c->findNextSblingFromCurrent("11"));
  // prev find
  EXPECT_TRUE(c->findPrevSiblingFromCurrent("9"));
  EXPECT_FALSE(c->findPrevSiblingFromCurrent("10"));
  EXPECT_FALSE(c->findPrevSiblingFromCurrent("11"));
}

TEST_F(TreeNodeTestSuit, SeekRoot)
{
  init();
  auto c = s_root->findChildRecursive("10");
  EXPECT_TRUE(c != nullptr);
  EXPECT_EQ("10", c->name());
  auto r = c->root();
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ("1", r->name());
  // removed
  auto removed = s_root->removeChild("4");
  r = c->root();
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ("4", r->name());

  s_root->pushChildAt("5", removed);
  check();
  r = c->root();
  EXPECT_TRUE(r != nullptr);
  EXPECT_EQ("1", r->name());
}

TEST_F(TreeNodeTestSuit, AccessByIterator)
{
  init();
  auto c = s_root->findChildRecursive("10");
  EXPECT_TRUE(c);
  EXPECT_EQ(c->name(), "10");
  auto it = *(c->iter());
  EXPECT_EQ(it->name(), "10");
}

// NOLINTEND
