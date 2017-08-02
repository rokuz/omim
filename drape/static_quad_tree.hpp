#pragma once

#include "geometry/rect2d.hpp"

#include "base/assert.hpp"
#include "base/stl_add.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace dp
{
uint8_t constexpr kChildrenCount = 4;

template <typename DataType, typename TraitsType, int Depth = 4>
class StaticQuadTree
{
public:
  struct Node
  {
    std::vector<DataType> m_data;
    std::array<std::unique_ptr<Node>, kChildrenCount> m_children;
#ifdef TREE_UNIT_TESTS
    uint8_t m_depth = 0;
    uint8_t m_x = 0;
    uint8_t m_y = 0;
#endif
  };
  using NodeHandler = std::function<void(std::unique_ptr<Node> const &)>;
  using DataHandler = std::function<void(DataType const &)>;

  explicit StaticQuadTree(size_t averageNodesCounts = 10)
  {
    m_root = CreateNode(0 /* depth */, averageNodesCounts);
  }

  void SetTraits(TraitsType const * traits)
  {
    m_traits = traits;
  }

  void Clear()
  {
    ForEachNode(m_root, [](std::unique_ptr<Node> const & node)
    {
      node->m_data.clear();
    });
  }

  bool Add(DataType const & data)
  {
    auto node = FindNodeForData(m_root, m_traits->GetRect(), data);
    if (node == nullptr)
      return false;
    node->m_data.push_back(data);
    return true;
  }

  bool Erase(DataType const & data)
  {
    auto node = FindNodeForData(m_root, m_traits->GetRect(), data);
    if (node == nullptr)
      return false;
    auto it = std::remove(node->m_data.begin(), node->m_data.end(), data);
    if (it == node->m_data.end())
      return false;
    node->m_data.erase(it, node->m_data.end());
    return true;
  }

  void ForEachInRect(m2::RectD const & rect, DataHandler const & handler) const
  {
    ForEachInRect(m_root, m_traits->GetRect(), rect, handler);
  }

#ifdef TREE_UNIT_TESTS
  Node const * GetNode(DataType const & data) const
  {
    return FindNodeForData(m_root, m_traits->GetRect(), data);
  }
#endif

private:
  std::unique_ptr<Node> CreateNode(uint8_t depth, size_t averageNodesCount)
  {
    if (depth > Depth)
      return nullptr;

    auto node = my::make_unique<Node>();
    node->m_data.reserve(averageNodesCount);

    for (uint8_t i = 0; i < static_cast<uint8_t>(node->m_children.size()); ++i)
    {
      node->m_children[i] = CreateNode(static_cast<uint8_t>(depth + 1), averageNodesCount);
#ifdef TREE_UNIT_TESTS
      if (node->m_children[i] != nullptr)
      {
        node->m_children[i]->m_depth = static_cast<uint8_t>(depth + 1);
        node->m_children[i]->m_x = static_cast<uint8_t>(i & 0x1);
        node->m_children[i]->m_y = static_cast<uint8_t>((i & 0x2) >> 1);
      }
#endif
    }

    return node;
  }

  void ForEachNode(std::unique_ptr<Node> const & node, NodeHandler const & handler) const
  {
    if (handler == nullptr || node == nullptr)
      return;

    handler(node);

    for (uint8_t i = 0; i < static_cast<uint8_t>(node->m_children.size()); ++i)
      ForEachNode(node->m_children[i], handler);
  }

  Node * FindNodeForData(std::unique_ptr<Node> const & node, m2::RectD const & rect,
                         DataType const & data) const
  {
    ASSERT(node != nullptr, ());
    ASSERT(m_traits != nullptr, ());

    auto const dataRect = m_traits->LimitRect(data);
    uint8_t intersectorCounter = 0;
    uint8_t intersectorIndex = 0;
    for (uint8_t i = 0; i < static_cast<uint8_t>(node->m_children.size()); ++i)
    {
      m2::RectD const childNodeRect = GetNodeRect(rect, i);
      if (childNodeRect.IsRectInside(dataRect))
      {
        if (node->m_children[i] != nullptr)
        {
          auto foundNode = FindNodeForData(node->m_children[i], childNodeRect, data);
          if (foundNode != nullptr)
            return foundNode;
        }
        else
        {
          return node.get();
        }
      }
      else if (childNodeRect.IsIntersect(dataRect))
      {
        intersectorCounter++;
        if (intersectorCounter == 1)
          intersectorIndex = i;
      }
    }

    // If the only child has intersection with data rect, let find node inside this child.
    if (intersectorCounter == 1)
    {
      if (node->m_children[intersectorIndex] == nullptr)
        return node.get();

      m2::RectD const childNodeRect = GetNodeRect(rect, intersectorIndex);
      auto foundNode = FindNodeForData(node->m_children[intersectorIndex], childNodeRect, data);
      if (foundNode != nullptr)
        return foundNode;
    }

    if (rect.IsIntersect(dataRect) || rect.IsRectInside(dataRect))
      return node.get();

    return nullptr;
  }

  void ForEachInRect(std::unique_ptr<Node> const & node, m2::RectD const & nodeRect,
                     m2::RectD const & targetRect, DataHandler const & handler) const
  {
    ASSERT(node != nullptr, ());
    ASSERT(m_traits != nullptr, ());

    for (auto const & data : node->m_data)
    {
      auto const dataRect = m_traits->LimitRect(data);
      if (dataRect.IsIntersect(targetRect) || targetRect.IsRectInside(dataRect))
        handler(data);
    }

    for (uint8_t i = 0; i < static_cast<uint8_t>(node->m_children.size()); ++i)
    {
      if (node->m_children[i] == nullptr)
        break;

      m2::RectD const childNodeRect = GetNodeRect(nodeRect, i);
      if (childNodeRect.IsIntersect(targetRect) || targetRect.IsRectInside(childNodeRect))
        ForEachInRect(node->m_children[i], childNodeRect, targetRect, handler);
    }
  }

  m2::RectD GetNodeRect(m2::RectD const & rect, uint8_t index) const
  {
    auto const x = static_cast<uint8_t>(index & 0x1);
    auto const y = static_cast<uint8_t>((index & 0x2) >> 1);
    double const sx = 0.5 * rect.SizeX();
    double const sy = 0.5 * rect.SizeY();
    double const minX = rect.minX() + x * sx;
    double const minY = rect.minY() + y * sy;
    return {minX, minY, minX + sx, minY + sy};
  }

  std::unique_ptr<Node> m_root;
  TraitsType const * m_traits = nullptr;
};
}  // namespace dp
