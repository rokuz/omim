#include "testing/testing.hpp"

#define TREE_UNIT_TESTS
#include "drape/static_quad_tree.hpp"

#include <vector>

namespace
{
struct Data
{
  Data(m2::RectD const & rect) : m_rect(rect) {}
  m2::RectD m_rect;

  bool operator == (Data const & data) const
  {
    return m_rect == data.m_rect;
  }
};

std::string DebugPrint(Data const & data)
{
  return DebugPrint(data.m_rect);
}

class Traits
{
public:
  m2::RectD const & LimitRect(Data const & data) const
  {
    return data.m_rect;
  }

  m2::RectD const & GetRect() const
  {
    return m_rect;
  }

  m2::RectD m_rect = m2::RectD(0.0, 0.0, 100.0, 100.0);
};
}  // namespace

UNIT_TEST(StaticQuadTree_Insertion)
{
  Traits traits;
  dp::StaticQuadTree<Data, Traits, 3> tree;
  tree.SetTraits(&traits);

  // Insertion to the root node.
  {
    auto const data = Data(m2::RectD(49.0, 49.0, 51.0, 51.0));
    TEST_EQUAL(tree.Add(data), true, ());
    auto node = tree.GetNode(data);
    TEST_EQUAL(node->m_x, 0, ());
    TEST_EQUAL(node->m_y, 0, ());
    TEST_EQUAL(node->m_depth, 0, ());
    TEST_EQUAL(node->m_data.front().m_rect, data.m_rect, ());
  }

  // Insertion to the children nodes.
  {
    auto const data = Data(m2::RectD(10.0, 74.0, 11.0, 76.0));
    TEST_EQUAL(tree.Add(data), true, ());
    auto node = tree.GetNode(data);
    TEST_EQUAL(node->m_x, 0, ());
    TEST_EQUAL(node->m_y, 1, ());
    TEST_EQUAL(node->m_depth, 1, ());
    TEST_EQUAL(node->m_data.front().m_rect, data.m_rect, ());
  }

  // Insertion to the deepest child.
  {
    auto const data = Data(m2::RectD(38.0, 91.0, 41.0, 93.0));
    TEST_EQUAL(tree.Add(data), true, ());
    auto node = tree.GetNode(data);
    TEST_EQUAL(node->m_x, 1, ());
    TEST_EQUAL(node->m_y, 1, ());
    TEST_EQUAL(node->m_depth, 3, ());
    TEST_EQUAL(node->m_data.front().m_rect, data.m_rect, ());
  }

  // Insertion out of rect.
  {
    auto const data = Data(m2::RectD(-10.0, -10.0, -5.0, -5.0));
    TEST_EQUAL(tree.Add(data), false, ());
  }

  // Insertion with intersection.
  {
    auto const data = Data(m2::RectD(-1.0, 50.0, 1.0, 51.0));
    TEST_EQUAL(tree.Add(data), true, ());
    auto node = tree.GetNode(data);
    TEST_EQUAL(node->m_x, 0, ());
    TEST_EQUAL(node->m_y, 0, ());
    TEST_EQUAL(node->m_depth, 0, ());
    TEST_EQUAL(node->m_data.back().m_rect, data.m_rect, ());
  }

  // Insertion with intersection (the only child intersection).
  {
    auto const data = Data(m2::RectD(-1.0, 99.0, 1.0, 100.0));
    TEST_EQUAL(tree.Add(data), true, ());
    auto node = tree.GetNode(data);
    TEST_EQUAL(node->m_x, 0, ());
    TEST_EQUAL(node->m_y, 1, ());
    TEST_EQUAL(node->m_depth, 3, ());
    TEST_EQUAL(node->m_data.back().m_rect, data.m_rect, ());
  }

  // Insertion with intersection (the only child intersection 2).
  {
    auto const data = Data(m2::RectD(12.0, 99.0, 13.0, 100.0));
    TEST_EQUAL(tree.Add(data), true, ());
    auto node = tree.GetNode(data);
    TEST_EQUAL(node->m_x, 0, ());
    TEST_EQUAL(node->m_y, 1, ());
    TEST_EQUAL(node->m_depth, 2, ());
    TEST_EQUAL(node->m_data.back().m_rect, data.m_rect, ());
  }
}

UNIT_TEST(StaticQuadTree_Erase)
{
  Traits traits;
  dp::StaticQuadTree<Data, Traits, 3> tree;
  tree.SetTraits(&traits);
  TEST_EQUAL(tree.Add(Data(m2::RectD(10.0, 74.0, 11.0, 76.0))), true, ());
  TEST_EQUAL(tree.Add(Data(m2::RectD(49.0, 49.0, 51.0, 51.0))), true, ());
  TEST_EQUAL(tree.Add(Data(m2::RectD(38.0, 91.0, 41.0, 93.0))), true, ());

  TEST_EQUAL(tree.Erase(Data(m2::RectD(10.0, 74.0, 11.0, 76.0))), true, ());
  TEST_EQUAL(tree.Erase(Data(m2::RectD(10.0, 74.0, 11.0, 76.0))), false, ());

  TEST_EQUAL(tree.Erase(Data(m2::RectD(49.0, 49.0, 51.0, 51.0))), true, ());

  TEST_EQUAL(tree.Erase(Data(m2::RectD(38.0, 91.0, 41.0, 93.0))), true, ());
}

UNIT_TEST(StaticQuadTree_ForEachInRect)
{
  Traits traits;
  dp::StaticQuadTree<Data, Traits, 3> tree;
  tree.SetTraits(&traits);

  std::vector<Data> data = {
    Data(m2::RectD(49.0, 49.0, 51.0, 51.0)), Data(m2::RectD(10.0, 74.0, 11.0, 76.0)),
    Data(m2::RectD(26.0, 76.0, 27.0, 77.0)), Data(m2::RectD(38.0, 91.0, 41.0, 93.0)),
    Data(m2::RectD(74.0, 76.0, 76.0, 77.0)),
  };

  TEST_EQUAL(tree.Add(data[0]), true, ());
  TEST_EQUAL(tree.Add(data[1]), true, ());
  TEST_EQUAL(tree.Add(data[2]), true, ());
  TEST_EQUAL(tree.Add(data[3]), true, ());
  TEST_EQUAL(tree.Add(data[4]), true, ());

  {
    std::vector<Data> expectedResult = {data[0], data[1], data[2], data[3]};
    std::vector<Data> result;
    tree.ForEachInRect(m2::RectD(0.0, 51.0, 49.0, 99.0), [&result](Data const &d)
    {
      result.push_back(d);
    });
    TEST_EQUAL(result, expectedResult, ());
  }

  {
    std::vector<Data> expectedResult = {data[2], data[3]};
    std::vector<Data> result;
    tree.ForEachInRect(m2::RectD(26.0, 76.0, 49.0, 99.0), [&result](Data const &d)
    {
      result.push_back(d);
    });
    TEST_EQUAL(result, expectedResult, ());
  }
}
