#include "kml/serdes_binary.hpp"

namespace kml
{
namespace binary
{
SerializerKml::SerializerKml(CategoryData & data, std::string const & pathToOriginalKml)
  : m_data(data)
  , m_pathToOriginalKml(pathToOriginalKml)
{
  ClearCollectionIndex();

  // Collect all strings and substitute each for index.
  auto const avgSz = data.m_bookmarksData.size() * 2 + data.m_tracksData.size() * 2 + 1;
  LocalizableStringCollector collector(avgSz);
  CollectorVisitor<decltype(collector)> visitor(collector);
  visitor(m_data);
  m_strings = collector.StealCollection();
}

SerializerKml::~SerializerKml()
{
  ClearCollectionIndex();
}

void SerializerKml::ClearCollectionIndex()
{
  LocalizableStringCollector collector(0);
  CollectorVisitor<decltype(collector)> clearVisitor(collector, true /* clear index */);
  clearVisitor(m_data);
}

DeserializerKml::DeserializerKml(CategoryData & data, std::string const & pathToOriginalKml)
  : m_data(data)
  , m_pathToOriginalKml(pathToOriginalKml)
{}
}  // namespace binary
}  // namespace kml
