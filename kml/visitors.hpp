#pragma once

#include "kml/types.hpp"

#include "coding/point_to_integer.hpp"
#include "coding/text_storage.hpp"
#include "coding/varint.hpp"

#include "base/bits.hpp"

#include <memory>
#include <vector>

namespace kml
{
template <typename Collector>
class CollectorVisitor
{
public:
  explicit CollectorVisitor(Collector & collector, bool clearIndex = false)
    : m_collector(collector)
    , m_clearIndex(clearIndex)
  {}

  template <typename T>
  void PerformAction(T & t)
  {
    if (m_clearIndex)
      t.ClearCollectionIndex();
    else
      t.Collect(m_collector);
  }

  void operator()(CategoryData & t, char const * /* name */ = nullptr)
  {
    PerformAction(t);
    t.Visit(*this);
  }

  void operator()(BookmarkData & t, char const * /* name */ = nullptr)
  {
    PerformAction(t);
    t.Visit(*this);
  }

  void operator()(TrackData & t, char const * /* name */ = nullptr)
  {
    PerformAction(t);
    t.Visit(*this);
  }

  template <typename T>
  void operator()(T & t, char const * /* name */ = nullptr) {}

  template <typename T>
  void operator()(std::vector<T> & vs, char const * /* name */ = nullptr)
  {
    for (auto const & v : vs)
      (*this)(v);
  }

  template <typename T>
  void operator()(std::vector<std::shared_ptr<T>> & vs, char const * /* name */ = nullptr)
  {
    for (auto const & v : vs)
      (*this)(*v);
  }

private:
  Collector & m_collector;
  bool const m_clearIndex;
};

namespace binary
{
template <typename Sink>
class CategorySerializerVisitor
{
public:
  explicit CategorySerializerVisitor(Sink & sink)
    : m_sink(sink)
  {}

  void operator()(CategoryData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(LocalizableStringIndex & index, char const * /* name */ = nullptr)
  {
    WriteVarUint(m_sink, static_cast<uint32_t>(index.size()));
    for (auto const & subIndex : index)
    {
      WriteVarUint(m_sink, static_cast<uint32_t>(subIndex.size()));
      for (auto const & p : subIndex)
      {
        (*this)(static_cast<uint8_t>(p.first));
        WriteVarUint(m_sink, p.second);
      }
    }
  }

  void operator()(bool b, char const * /* name */ = nullptr)
  {
    (*this)(static_cast<uint8_t>(b));
  }

  template <typename D>
  std::enable_if_t<std::is_integral<D>::value>
  operator()(D d, char const * /* name */ = nullptr)
  {
    WriteToSink(m_sink, d);
  }

  template <typename T>
  void operator()(T & t, char const * /* name */ = nullptr) {}

private:
  Sink & m_sink;
};

template <typename Sink>
class BookmarkSerializerVisitor
{
public:
  explicit BookmarkSerializerVisitor(Sink & sink)
    : m_sink(sink)
  {}

  void operator()(CategoryData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(BookmarkData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(TrackData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(LocalizableStringIndex & index, char const * /* name */ = nullptr)
  {
    WriteVarUint(m_sink, static_cast<uint32_t>(index.size()));
    for (auto const & subIndex : index)
    {
      WriteVarUint(m_sink, static_cast<uint32_t>(subIndex.size()));
      for (auto const & p : subIndex)
      {
        (*this)(static_cast<uint8_t>(p.first));
        WriteVarUint(m_sink, p.second);
      }
    }
  }

  void operator()(LocalizableString const & str, char const * /* name */ = nullptr) {}

  void operator()(bool b, char const * /* name */ = nullptr)
  {
    (*this)(static_cast<uint8_t>(b));
  }

  void operator()(m2::PointD const & pt, char const * /* name */ = nullptr)
  {
    uint64_t const encoded = bits::ZigZagEncode(PointToInt64(pt, POINT_COORD_BITS));
    WriteVarUint(m_sink, encoded);
  }

  void operator()(double d, char const * /* name */ = nullptr)
  {
    uint64_t const encoded = DoubleToUint32(d, kMinLineWidth, kMaxLineWidth, 30 /* coordBits */);
    WriteVarUint(m_sink, encoded);
  }

  void operator()(Timestamp const & t, char const * name = nullptr)
  {
    WriteVarUint(m_sink, ToSecondsSinceEpoch(t));
  }

  void operator()(PredefinedColor color, char const * /* name */ = nullptr)
  {
    (*this)(static_cast<uint8_t>(color));
  }

  template <typename T>
  void operator()(std::vector<T> const & vs, char const * /* name */ = nullptr)
  {
    WriteVarUint(m_sink, static_cast<uint32_t>(vs.size()));
    for (auto const & v : vs)
      (*this)(v);
  }

  template <typename T>
  void operator()(std::vector<std::shared_ptr<T>> & vs, char const * /* name */ = nullptr)
  {
    WriteVarUint(m_sink, static_cast<uint32_t>(vs.size()));
    for (auto const & v : vs)
      (*this)(*v);
  }

  template <typename D>
  std::enable_if_t<std::is_integral<D>::value>
  operator()(D d, char const * /* name */ = nullptr)
  {
    WriteToSink(m_sink, d);
  }

  template <typename R>
  std::enable_if_t<!std::is_integral<R>::value>
  operator()(R const & r, char const * /* name */ = nullptr)
  {
    r.Visit(*this);
  }

private:
  Sink & m_sink;
};

template <typename Source>
class CategoryDeserializerVisitor
{
public:
  explicit CategoryDeserializerVisitor(Source & source)
    : m_source(source)
  {}

  void operator()(CategoryData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(LocalizableStringIndex & index, char const * /* name */ = nullptr)
  {
    auto const indexSize = ReadVarUint<uint32_t, Source>(m_source);
    index.reserve(indexSize);
    for (uint32_t i = 0; i < indexSize; ++i)
    {
      index.emplace_back(std::unordered_map<uint8_t, uint32_t>());
      auto const subIndexSize = ReadVarUint<uint32_t, Source>(m_source);
      for (uint32_t j = 0; j < subIndexSize; ++j)
      {
        auto const lang = ReadPrimitiveFromSource<uint8_t>(m_source);
        auto const strIndex = ReadVarUint<uint32_t, Source>(m_source);
        index.back()[lang] = strIndex;
      }
    }
  }

  void operator()(bool & b, char const * /* name */ = nullptr)
  {
    b = static_cast<bool>(ReadPrimitiveFromSource<uint8_t>(m_source));
  }

  template <typename D>
  std::enable_if_t<std::is_integral<D>::value>
  operator()(D & d, char const * /* name */ = nullptr)
  {
    d = ReadPrimitiveFromSource<D>(m_source);
  }

  template <typename T>
  void operator()(T & t, char const * /* name */ = nullptr) {}

private:
  Source & m_source;
};

template <typename Source>
class BookmarkDeserializerVisitor
{
public:
  explicit BookmarkDeserializerVisitor(Source & source)
    : m_source(source)
  {}

  void operator()(CategoryData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(BookmarkData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(TrackData & t, char const * /* name */ = nullptr)
  {
    t.Visit(*this);
  }

  void operator()(LocalizableStringIndex & index, char const * /* name */ = nullptr)
  {
    auto const indexSize = ReadVarUint<uint32_t, Source>(m_source);
    index.reserve(indexSize);
    for (uint32_t i = 0; i < indexSize; ++i)
    {
      index.emplace_back(std::unordered_map<uint8_t, uint32_t>());
      auto const subIndexSize = ReadVarUint<uint32_t, Source>(m_source);
      for (uint32_t j = 0; j < subIndexSize; ++j)
      {
        auto const lang = ReadPrimitiveFromSource<uint8_t>(m_source);
        auto const strIndex = ReadVarUint<uint32_t, Source>(m_source);
        index.back()[lang] = strIndex;
      }
    }
  }

  void operator()(LocalizableString & str, char const * /* name */ = nullptr) {}

  void operator()(bool & b, char const * /* name */ = nullptr)
  {
    b = static_cast<bool>(ReadPrimitiveFromSource<uint8_t>(m_source));
  }

  void operator()(m2::PointD & pt, char const * /* name */ = nullptr)
  {
    auto const v = ReadVarUint<uint64_t, Source>(m_source);
    pt = Int64ToPoint(bits::ZigZagDecode(v), POINT_COORD_BITS);
  }

  void operator()(double & d, char const * /* name */ = nullptr)
  {
    auto const v = ReadVarUint<uint32_t, Source>(m_source);
    d = Uint32ToDouble(v, kMinLineWidth, kMaxLineWidth, 30 /* coordBits */);
  }

  void operator()(Timestamp & t, char const * name = nullptr)
  {
    auto const v = ReadVarUint<uint64_t, Source>(m_source);
    t = FromSecondsSinceEpoch(v);
  }

  void operator()(PredefinedColor & color, char const * /* name */ = nullptr)
  {
    color = static_cast<PredefinedColor>(ReadPrimitiveFromSource<uint8_t>(m_source));
  }

  template <typename T>
  void operator()(std::vector<T> & vs, char const * /* name */ = nullptr)
  {
    auto const sz = ReadVarUint<uint32_t, Source>(m_source);
    vs.reserve(sz);
    for (uint32_t i = 0; i < sz; ++i)
    {
      vs.emplace_back(T());
      (*this)(vs.back());
    }
  }

  template <typename T>
  void operator()(std::vector<std::shared_ptr<T>> & vs, char const * /* name */ = nullptr)
  {
    auto const sz = ReadVarUint<uint32_t, Source>(m_source);
    vs.reserve(sz);
    for (uint32_t i = 0; i < sz; ++i)
    {
      vs.emplace_back(std::make_shared<T>());
      (*this)(*vs.back());
    }
  }

  template <typename D>
  std::enable_if_t<std::is_integral<D>::value>
  operator()(D & d, char const * /* name */ = nullptr)
  {
    d = ReadPrimitiveFromSource<D>(m_source);
  }

  template <typename R>
  std::enable_if_t<!std::is_integral<R>::value>
  operator()(R & r, char const * /* name */ = nullptr)
  {
    r.Visit(*this);
  }

private:
  Source & m_source;
};

template <typename Reader>
class DeserializedStringCollector
{
public:
  explicit DeserializedStringCollector(coding::BlockedTextStorage<Reader> & textStorage)
    : m_textStorage(textStorage)
  {}

  template <typename... OtherStrings>
  void Collect(LocalizableStringIndex & index,
               LocalizableString & str,
               OtherStrings & ... args)
  {
    if (m_lastIndex != &index)
    {
      m_counter = 0;
      m_lastIndex = &index;
    }

    if (m_counter >= index.size())
      return;

    auto subIndex = index[m_counter];
    auto const stringsCount = m_textStorage.GetNumStrings();
    for (auto const & p : subIndex)
    {
      str[p.first] = (p.second < stringsCount) ?
                     m_textStorage.ExtractString(p.second) : "";
    }

    m_counter++;
    Collect(index, args...);
  }

  template <typename...>
  void Collect(LocalizableStringIndex & index) {}

private:
  coding::BlockedTextStorage<Reader> & m_textStorage;
  LocalizableStringIndex * m_lastIndex = nullptr;
  size_t m_counter = 0;
};

}  // namespace binary
}  // namespace kml
