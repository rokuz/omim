#pragma once

#include "kml/types.hpp"

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
  SerializerKml(CategoryData & data)
    : m_data(data)
  {}

  template <typename Sink>
  void Serialize(Sink & sink)
  {
    WriteToSink(sink, Version::Latest);

    auto const startPos = sink.Pos();

    /*HeaderV0 header;
    WriteZeroesToSink(sink, header.Size());

    header.m_keysOffset = sink.Pos() - startPos;
    SerializeTranslationKeys(sink);

    std::vector<UGCOffset> ugcOffsets;

    header.m_ugcsOffset = sink.Pos() - startPos;
    SerializeUGC(sink, ugcOffsets);

    header.m_indexOffset = sink.Pos() - startPos;
    SerializeIndex(sink, ugcOffsets);

    header.m_textsOffset = sink.Pos() - startPos;
    SerializeTexts(sink);

    header.m_eosOffset = sink.Pos() - startPos;
    sink.Seek(startPos);
    header.Serialize(sink);
    sink.Seek(startPos + header.m_eosOffset);*/
  }

private:
  CategoryData & m_data;
};
}  // namespace binary
}  // namespace kml
