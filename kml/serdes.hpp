#pragma once

#include "kml/types.hpp"

#include "coding/parse_xml.hpp"
#include "coding/reader.hpp"

#include "base/exception.hpp"
#include "base/stl_add.hpp"

#include <chrono>
#include <string>

namespace kml
{
class KmlParser
{
public:
  explicit KmlParser(CategoryData & data);
  bool Push(std::string const & name);
  void AddAttr(std::string const & attr, std::string const & value);
  bool IsValidAttribute(std::string const & type, std::string const & value,
                        std::string const & attrInLowerCase) const;
  std::string const & GetTagFromEnd(size_t n) const;
  void Pop(std::string const & tag);
  void CharData(std::string value);

private:
  enum GeometryType
  {
    GEOMETRY_TYPE_UNKNOWN,
    GEOMETRY_TYPE_POINT,
    GEOMETRY_TYPE_LINE
  };

  void Reset();
  bool ParsePoint(std::string const & s, char const * delim, m2::PointD & pt);
  void SetOrigin(std::string const & s);
  void ParseLineCoordinates(std::string const & s, char const * blockSeparator,
                            char const * coordSeparator);
  bool MakeValid();
  void ParseColor(std::string const &value);
  bool GetColorForStyle(std::string const & styleUrl, uint32_t & color);

  CategoryData & m_data;

  std::vector<std::string> m_tags;
  GeometryType m_geometryType;
  std::vector<m2::PointD> m_points;
  uint32_t m_color;

  std::string m_styleId;
  std::string m_mapStyleId;
  std::string m_styleUrlKey;
  std::map<std::string, uint32_t> m_styleUrl2Color;
  std::map<std::string, std::string> m_mapStyle2Style;

  std::string m_name;
  std::string m_description;
  PredefinedColor m_predefinedColor;
  Timestamp m_timestamp;
  m2::PointD m_org;
  uint8_t m_viewportScale;
};

template <typename ReaderType>
class DeserializerKml
{
public:
  DECLARE_EXCEPTION(DeserializeException, RootException);

  template <typename... ReaderArgs>
  explicit DeserializerKml(ReaderArgs && ... args)
    : m_src(std::make_unique<ReaderType>(std::forward<ReaderArgs>(args)...))
  {}

  void operator()(CategoryData & categoryData)
  {
    KmlParser parser(categoryData);
    if (!ParseXML(m_src, parser, true))
      MYTHROW(DeserializeException, ("Could not parse KML."));
  }

private:
  ReaderSource<ReaderPtr<ReaderType>> m_src;
};
}  // namespace kml
