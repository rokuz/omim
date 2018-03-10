#pragma once

#include "geometry/point2d.hpp"

#include <cstdint>
#include <chrono>
#include <ctime>
#include <memory>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace kml
{
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
using LocalizableString = std::unordered_map<uint8_t, std::string>;
using LocalizableStringIndex = std::vector<std::unordered_map<uint8_t, uint32_t>>;

inline uint64_t ToSecondsSinceEpoch(Timestamp const & time)
{
  auto const s = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch());
  return static_cast<uint64_t>(s.count());
}

inline Timestamp FromSecondsSinceEpoch(uint64_t seconds)
{
  return Timestamp(std::chrono::seconds(seconds));
}

template <typename T>
inline bool Compare(std::vector<std::shared_ptr<T>> const & v1,
                    std::vector<std::shared_ptr<T>> const & v2)
{
  if (v1.size() != v2.size())
    return false;

  for (size_t i = 0; i < v1.size(); ++i)
  {
    if (*v1[i] != *v2[i])
      return false;
  }

  return true;
}

inline bool Compare(std::vector<m2::PointD> const & v1,
                    std::vector<m2::PointD> const & v2)
{
  if (v1.size() != v2.size())
    return false;

  double constexpr kEps = 1e-5;
  for (size_t i = 0; i < v1.size(); ++i)
  {
    if (!v1[i].EqualDxDy(v2[i], kEps))
      return false;
  }

  return true;
}

inline bool Compare(Timestamp const & ts1, Timestamp const & ts2)
{
  return ToSecondsSinceEpoch(ts1) == ToSecondsSinceEpoch(ts2);
}

uint32_t constexpr kEmptyStringId = 0;
double constexpr kMinLineWidth = 0.0;
double constexpr kMaxLineWidth = 100.0;

class LocalizableStringCollector
{
public:
  explicit LocalizableStringCollector(size_t reservedCollectionSize)
  {
    m_collection.reserve(reservedCollectionSize + 1);
    m_collection.emplace_back(std::string());
  }

  template <typename... OtherStrings>
  void Collect(LocalizableStringIndex & index,
               LocalizableString const & str,
               OtherStrings const & ... args)
  {
    index.emplace_back(std::unordered_map<uint8_t, uint32_t>());
    for (auto const & p : str)
    {
      if (p.second.empty())
      {
        index.back().insert(std::make_pair(p.first, kEmptyStringId));
        continue;
      }
      index.back().insert(std::make_pair(p.first, m_counter));
      m_counter++;
      m_collection.push_back(p.second);
    }
    Collect(index, args...);
  }

  template <typename...>
  void Collect(LocalizableStringIndex & index) {}

  std::vector<std::string> && StealCollection() { return std::move(m_collection); }

private:
  uint32_t m_counter = kEmptyStringId + 1;
  std::vector<std::string> m_collection;
};

#define DECLARE_COLLECTABLE(IndexType, ...)            \
  IndexType m_collectionIndex;                         \
  template <typename Collector>                        \
  void Collect(Collector & collector)                  \
  {                                                    \
    collector.Collect(m_collectionIndex, __VA_ARGS__); \
  }                                                    \
  void ClearCollectionIndex()                          \
  {                                                    \
    m_collectionIndex.clear();                         \
  }                                                    \

#define VISITOR_COLLECTABLE visitor(m_collectionIndex, "collectionIndex")

inline std::string DebugPrint(LocalizableString const & str)
{
  std::ostringstream os;
  os << "[";
  for (auto it = str.cbegin(); it != str.end();)
  {
    os << static_cast<uint32_t>(it->first) << ": " << it->second;
    ++it;
    if (it != str.end())
      os << ", ";
  }
  os << "]";
  return os.str();
}

inline std::string DebugPrint(Timestamp const & ts)
{
  auto t = std::chrono::system_clock::to_time_t(ts);
  return std::ctime(&t);
}

template <typename T>
inline std::string DebugPrint(std::vector<std::shared_ptr<T>> const & v)
{
  std::ostringstream os;
  os << "[";
  for (auto const & elem : v)
    os << DebugPrint(*elem);
  os << "]";
  return os.str();
}

inline std::string DebugPrint(m2::PointD const & pt)
{
  std::ostringstream os;
  os << "[" << pt.x << ", " << pt.y << "]";
  return os.str();
}
}  // namespace kml
