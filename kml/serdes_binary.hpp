#pragma once

#include "kml/header_binary.hpp"
#include "kml/types.hpp"
#include "kml/visitors.hpp"

#include "coding/read_write_utils.hpp"
#include "coding/sha1.hpp"
#include "coding/text_storage.hpp"

#include "platform/platform.hpp"

#include <string>
#include <vector>

namespace kml
{
namespace binary
{
enum class Version : uint8_t
{
  V0 = 0,
  Latest = V0
};

class SerializerKml
{
public:
  SerializerKml(CategoryData & data, std::string const & pathToOriginalKml);
  ~SerializerKml();

  void ClearCollectionIndex();

  template <typename Sink>
  void Serialize(Sink & sink)
  {
    // Write format version.
    WriteToSink(sink, Version::Latest);

    // Write SHA1 of original KML file.
    coding::SHA1::Hash hash = {};
    if (GetPlatform().IsFileExistsByFullPath(m_pathToOriginalKml))
      hash = coding::SHA1::Calculate(m_pathToOriginalKml);
    rw::WriteVectorOfPOD(sink, hash);

    auto const startPos = sink.Pos();

    // Reserve place for the header.
    Header header;
    WriteZeroesToSink(sink, header.Size());

    // Serialize category.
    header.m_categoryOffset = sink.Pos() - startPos;
    SerializeCategory(sink);

    // Serialize bookmarks.
    header.m_bookmarksOffset = sink.Pos() - startPos;
    SerializeBookmarks(sink);

    // Serialize tracks.
    header.m_tracksOffset = sink.Pos() - startPos;
    SerializeTracks(sink);

    // Serialize strings.
    header.m_stringsOffset = sink.Pos() - startPos;
    SerializeStrings(sink);

    // Fill header.
    header.m_eosOffset = sink.Pos() - startPos;
    sink.Seek(startPos);
    header.Serialize(sink);
    sink.Seek(startPos + header.m_eosOffset);
  }

  template <typename Sink>
  void SerializeCategory(Sink & sink)
  {
    CategorySerializerVisitor<Sink> visitor(sink);
    visitor(m_data);
  }

  template <typename Sink>
  void SerializeBookmarks(Sink & sink)
  {
    BookmarkSerializerVisitor<Sink> visitor(sink);
    visitor(m_data.m_bookmarksData);
  }

  template <typename Sink>
  void SerializeTracks(Sink & sink)
  {
    BookmarkSerializerVisitor<Sink> visitor(sink);
    visitor(m_data.m_tracksData);
  }

  // Serializes texts in a compressed storage with block access.
  template <typename Sink>
  void SerializeStrings(Sink & sink)
  {
    coding::BlockedTextStorageWriter<Sink> writer(sink, 200000 /* blockSize */);
    for (auto const & str : m_strings)
      writer.Append(str);
  }

private:
  CategoryData & m_data;
  std::string const m_pathToOriginalKml;
  std::vector<std::string> m_strings;
};

class DeserializerKml
{
public:
  DECLARE_EXCEPTION(DeserializeException, RootException);

  DeserializerKml(CategoryData & data, std::string const & pathToOriginalKml);

  template <typename R>
  void Deserialize(R & reader)
  {
    // Check version.
    NonOwningReaderSource source(reader);
    auto const v = ReadPrimitiveFromSource<Version>(source);
    if (v != Version::Latest)
      MYTHROW(DeserializeException, ("Incorrect file version."));

    // Check hash.
    coding::SHA1::Hash hash = {};
    std::vector<uint8_t> hashData;
    rw::ReadVectorOfPOD(source, hashData);
    ASSERT_EQUAL(hashData.size(), hash.size(), ());
    std::copy(std::begin(hashData), std::end(hashData), std::begin(hash));
    if (GetPlatform().IsFileExistsByFullPath(m_pathToOriginalKml))
    {
      auto const currentHash = coding::SHA1::Calculate(m_pathToOriginalKml);
      if (currentHash != hash)
        MYTHROW(DeserializeException, ("Binary KML is obsolete."));
    }

    auto subReader = reader.CreateSubReader(source.Pos(), source.Size());
    InitializeIfNeeded(*subReader);

    // Deserialize category.
    {
      auto categorySubReader = CreateCategorySubReader(*subReader);
      NonOwningReaderSource src(*categorySubReader);
      CategoryDeserializerVisitor<decltype(src)> visitor(src);
      visitor(m_data);
    }

    // Deserialize bookmarks.
    {
      auto bookmarkSubReader = CreateBookmarkSubReader(*subReader);
      NonOwningReaderSource src(*bookmarkSubReader);
      BookmarkDeserializerVisitor<decltype(src)> visitor(src);
      visitor(m_data.m_bookmarksData);
    }

    // Deserialize tracks.
    {
      auto trackSubReader = CreateTrackSubReader(*subReader);
      NonOwningReaderSource src(*trackSubReader);
      BookmarkDeserializerVisitor<decltype(src)> visitor(src);
      visitor(m_data.m_tracksData);
    }

    // Deserialize strings.
    {
      auto textsSubReader = CreateStringsSubReader(*subReader);
      coding::BlockedTextStorage<Reader> strings(*textsSubReader);
      DeserializedStringCollector<Reader> collector(strings);
      CollectorVisitor<decltype(collector)> visitor(collector);
      visitor(m_data);
      CollectorVisitor<decltype(collector)> clearVisitor(collector,
                                                         true /* clear index */);
      clearVisitor(m_data);
    }
  }

private:
  template <typename Reader>
  void InitializeIfNeeded(Reader & reader)
  {
    if (m_initialized)
      return;

    NonOwningReaderSource source(reader);
    m_header.Deserialize(source);
    m_initialized = true;
  }

  template <typename R>
  std::unique_ptr<Reader> CreateCategorySubReader(R & reader)
  {
    ASSERT(m_initialized, ());

    auto const pos = m_header.m_categoryOffset;
    ASSERT_GREATER_OR_EQUAL(m_header.m_bookmarksOffset, pos, ());
    auto const size = m_header.m_bookmarksOffset - pos;
    return reader.CreateSubReader(pos, size);
  }

  template <typename R>
  std::unique_ptr<Reader> CreateBookmarkSubReader(R & reader)
  {
    ASSERT(m_initialized, ());

    auto const pos = m_header.m_bookmarksOffset;
    ASSERT_GREATER_OR_EQUAL(m_header.m_tracksOffset, pos, ());
    auto const size = m_header.m_tracksOffset - pos;
    return reader.CreateSubReader(pos, size);
  }

  template <typename R>
  std::unique_ptr<Reader> CreateTrackSubReader(R & reader)
  {
    ASSERT(m_initialized, ());

    auto const pos = m_header.m_tracksOffset;
    ASSERT_GREATER_OR_EQUAL(m_header.m_stringsOffset, pos, ());
    auto const size = m_header.m_stringsOffset - pos;
    return reader.CreateSubReader(pos, size);
  }

  template <typename R>
  std::unique_ptr<Reader> CreateStringsSubReader(R & reader)
  {
    ASSERT(m_initialized, ());

    auto const pos = m_header.m_stringsOffset;
    ASSERT_GREATER_OR_EQUAL(m_header.m_eosOffset, pos, ());
    auto const size = m_header.m_eosOffset - pos;
    return reader.CreateSubReader(pos, size);
  }

  CategoryData & m_data;
  std::string const m_pathToOriginalKml;
  Header m_header;

  bool m_initialized = false;
};
}  // namespace binary
}  // namespace kml
