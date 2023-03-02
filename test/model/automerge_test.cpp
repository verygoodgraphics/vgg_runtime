#include <gtest/gtest.h>

#include "Automerge.h"

class VggAutomergeTestSuite : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }
  void TearDown() override
  {
  }
};

Automerge quickstart();

TEST_F(VggAutomergeTestSuite, Smoke)
{
  quickstart();
}

Automerge quickstart()
{
  Automerge doc1;
  auto result = doc1.transact_with(
    [](const std::vector<ExId>& result)
    { return CommitOptions<OpObserver>(std::string("Add card"), {}, {}); },
    [](Transaction& tx) -> std::vector<ExId>
    {
      ExId cards = tx.put_object(ExId(), Prop("cards"), ObjType::List);
      ExId card1 = tx.insert_object(cards, 0, ObjType::Map);
      tx.put(card1,
             Prop("title"),
             ScalarValue{ ScalarValue::Str, std::string("Rewrite everything in Clojure") });
      tx.put(card1, Prop("done"), ScalarValue{ ScalarValue::Boolean, false });

      ExId card2 = tx.insert_object(cards, 0, ObjType::Map);
      tx.put(card2,
             Prop("title"),
             ScalarValue{ ScalarValue::Str, std::string("Rewrite everything in Haskell") });
      tx.put(card2, Prop("done"), ScalarValue{ ScalarValue::Boolean, false });

      return { cards, card1 };
    });
  ExId& cards = result.first[0];
  ExId& card1 = result.first[1];

  Automerge doc3;
  doc3.merge(doc1);

  auto binary = doc1.save();
  Automerge doc2 = Automerge::load({ binary.cbegin(), binary.size() });

  doc1.transact_with(
    [](const std::vector<ExId>& result)
    { return CommitOptions<OpObserver>(std::string("Mark card as done"), {}, {}); },
    [&](Transaction& tx) -> std::vector<ExId>
    {
      tx.put(card1, Prop("done"), ScalarValue{ ScalarValue::Boolean, true });

      return {};
    });

  doc2.transact_with([](const std::vector<ExId>& result)
                     { return CommitOptions<OpObserver>(std::string("Delete card"), {}, {}); },
                     [&](Transaction& tx) -> std::vector<ExId>
                     {
                       tx.delete_(cards, Prop(0));

                       return {};
                     });

  doc1.merge(doc2);

  assert(json(doc1) == json::parse(R"({
    "cards": [
        {
            "title": "Rewrite everything in Clojure",
            "done": true
        }
    ]
})"));
  assert(json(doc2) == json::parse(R"({
    "cards": [
        {
            "title": "Rewrite everything in Clojure",
            "done": false
        }
    ]
})"));

  assert(json(doc3) == json::parse(R"({
    "cards": [
        {
            "title": "Rewrite everything in Haskell",
            "done": false
        },
        {
            "title": "Rewrite everything in Clojure",
            "done": false
        }
    ]
})"));

  return doc3;
}