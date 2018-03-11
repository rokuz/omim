#include "kml/serdes.hpp"

#include "geometry/mercator.hpp"

#include "coding/hex.hpp"
#include "coding/multilang_utf8_string.hpp"

#include "base/assert.hpp"
#include "base/string_utils.hpp"
#include "base/timer.hpp"

#include <sstream>

using namespace std::string_literals;

namespace kml
{
namespace
{
std::string const kPlacemark = "Placemark";
std::string const kStyle = "Style";
std::string const kDocument = "Document";
std::string const kStyleMap = "StyleMap";
std::string const kStyleUrl = "styleUrl";
std::string const kPair = "Pair";

auto const kDefaultLang = StringUtf8Multilang::kDefaultCode;

std::string PointToString(m2::PointD const & org)
{
  double const lon = MercatorBounds::XToLon(org.x);
  double const lat = MercatorBounds::YToLat(org.y);

  std::ostringstream ss;
  ss.precision(8);

  ss << lon << "," << lat;
  return ss.str();
}

PredefinedColor ExtractPlacemarkPredefinedColor(std::string const & s)
{
  if (s == "#placemark-red")
    return PredefinedColor::Red;
  else if (s == "#placemark-blue")
    return PredefinedColor::Blue;
  else if (s == "#placemark-purple")
    return PredefinedColor::Purple;
  else if (s == "#placemark-yellow")
    return PredefinedColor::Yellow;
  else if (s == "#placemark-pink")
    return PredefinedColor::Pink;
  else if (s == "#placemark-brown")
    return PredefinedColor::Brown;
  else if (s == "#placemark-green")
    return PredefinedColor::Green;
  else if (s == "#placemark-orange")
    return PredefinedColor::Orange;

  // Default color.
  return PredefinedColor::Red;
}

template <typename Channel>
uint32_t ToRGBA(Channel red, Channel green, Channel blue, Channel alpha)
{
  return static_cast<uint8_t>(red) << 24 | static_cast<uint8_t>(green) << 16 |
         static_cast<uint8_t>(blue) << 8 | static_cast<uint8_t>(alpha);
}
}  // namespace

KmlParser::KmlParser(CategoryData & data) : m_data(data) { Reset(); }

void KmlParser::Reset()
{
  m_name.clear();
  m_description.clear();
  m_org = {};
  m_predefinedColor = PredefinedColor::None;
  m_viewportScale = 0;
  m_timestamp = {};

  m_color = 0;
  m_styleId.clear();
  m_mapStyleId.clear();
  m_styleUrlKey.clear();

  m_points.clear();
  m_geometryType = GEOMETRY_TYPE_UNKNOWN;
}

bool KmlParser::ParsePoint(std::string const & s, char const * delim, m2::PointD & pt)
{
  // Order in string is: lon, lat, z.
  strings::SimpleTokenizer iter(s, delim);
  if (iter)
  {
    double lon;
    if (strings::to_double(*iter, lon) && MercatorBounds::ValidLon(lon) && ++iter)
    {
      double lat;
      if (strings::to_double(*iter, lat) && MercatorBounds::ValidLat(lat))
      {
        pt = MercatorBounds::FromLatLon(lat, lon);
        return true;
      }
    }
  }

  return false;
}

void KmlParser::SetOrigin(std::string const & s)
{
  m_geometryType = GEOMETRY_TYPE_POINT;

  m2::PointD pt;
  if (ParsePoint(s, ", \n\r\t", pt))
    m_org = pt;
}

void KmlParser::ParseLineCoordinates(std::string const & s, char const * blockSeparator,
                                     char const * coordSeparator)
{
  m_geometryType = GEOMETRY_TYPE_LINE;

  strings::SimpleTokenizer tupleIter(s, blockSeparator);
  while (tupleIter)
  {
    m2::PointD pt;
    if (ParsePoint(*tupleIter, coordSeparator, pt))
    {
      if (m_points.size() == 0 || !(pt - m_points.back()).IsAlmostZero())
        m_points.push_back(std::move(pt));
    }
    ++tupleIter;
  }
}

bool KmlParser::MakeValid()
{
  if (GEOMETRY_TYPE_POINT == m_geometryType)
  {
    if (MercatorBounds::ValidX(m_org.x) && MercatorBounds::ValidY(m_org.y))
    {
      // Set default name.
      if (m_name.empty())
        m_name = PointToString(m_org);

      // Set default pin.
      if (m_predefinedColor == PredefinedColor::None)
        m_predefinedColor = PredefinedColor::Red;

      return true;
    }
    return false;
  }
  else if (GEOMETRY_TYPE_LINE == m_geometryType)
  {
    return m_points.size() > 1;
  }

  return false;
}

void KmlParser::ParseColor(std::string const & value)
{
  auto const fromHex = FromHex(value);
  if (fromHex.size() != 4)
    return;

  // Color positions in HEX – aabbggrr.
  m_color = ToRGBA(fromHex[3], fromHex[2], fromHex[1], fromHex[0]);
}

bool KmlParser::GetColorForStyle(std::string const & styleUrl, uint32_t & color)
{
  if (styleUrl.empty())
    return false;

  // Remove leading '#' symbol
  auto const it = m_styleUrl2Color.find(styleUrl.substr(1));
  if (it != m_styleUrl2Color.end())
  {
    color = it->second;
    return true;
  }
  return false;
}

bool KmlParser::Push(std::string const & name)
{
  m_tags.push_back(name);
  return true;
}

void KmlParser::AddAttr(std::string const & attr, std::string const & value)
{
  std::string attrInLowerCase = attr;
  strings::AsciiToLower(attrInLowerCase);

  if (IsValidAttribute(kStyle, value, attrInLowerCase))
    m_styleId = value;
  else if (IsValidAttribute(kStyleMap, value, attrInLowerCase))
    m_mapStyleId = value;
}

bool KmlParser::IsValidAttribute(std::string const & type, std::string const & value,
                                 std::string const & attrInLowerCase) const
{
  return (GetTagFromEnd(0) == type && !value.empty() && attrInLowerCase == "id");
}

std::string const & KmlParser::GetTagFromEnd(size_t n) const
{
  ASSERT_LESS(n, m_tags.size(), ());
  return m_tags[m_tags.size() - n - 1];
}

void KmlParser::Pop(std::string const & tag)
{
  ASSERT_EQUAL(m_tags.back(), tag, ());

  if (tag == kPlacemark)
  {
    if (MakeValid())
    {
      if (GEOMETRY_TYPE_POINT == m_geometryType)
      {
        auto data = std::make_shared<BookmarkData>();
        data->m_name[kDefaultLang] = m_name;
        data->m_description[kDefaultLang] = m_description;
        data->m_color.m_predefinedColor = m_predefinedColor;
        data->m_color.m_rgba = m_color;
        data->m_viewportScale = m_viewportScale;
        data->m_timestamp = m_timestamp;
        data->m_point = m_org;
        m_data.m_bookmarksData.emplace_back(std::move(data));
      }
      else if (GEOMETRY_TYPE_LINE == m_geometryType)
      {
        auto data = std::make_shared<TrackData>();
        data->m_name[kDefaultLang] = m_name;
        data->m_description[kDefaultLang] = m_description;
        TrackLayer layer;
        layer.m_color.m_predefinedColor = PredefinedColor::None;
        layer.m_color.m_rgba = m_color;
        data->m_layers.emplace_back(std::move(layer));
        data->m_timestamp = m_timestamp;
        data->m_points = m_points;
        m_data.m_tracksData.emplace_back(std::move(data));
      }
    }
    Reset();
  }
  else if (tag == kStyle)
  {
    if (GetTagFromEnd(1) == kDocument)
    {
      if (!m_styleId.empty())
      {
        m_styleUrl2Color[m_styleId] = m_color;
        m_color = 0;
      }
    }
  }

  m_tags.pop_back();
}

void KmlParser::CharData(std::string value)
{
  strings::Trim(value);

  size_t const count = m_tags.size();
  if (count > 1 && !value.empty())
  {
    std::string const & currTag = m_tags[count - 1];
    std::string const & prevTag = m_tags[count - 2];
    std::string const ppTag = count > 2 ? m_tags[count - 3] : std::string();

    if (prevTag == kDocument)
    {
      if (currTag == "name")
        m_data.m_name[kDefaultLang] = value;
      else if (currTag == "visibility")
        m_data.m_visible = value != "0";
    }
    else if (prevTag == kPlacemark)
    {
      if (currTag == "name")
      {
        m_name = value;
      }
      else if (currTag == "styleUrl")
      {
        // Bookmark draw style.
        m_predefinedColor = ExtractPlacemarkPredefinedColor(value);

        // Check if url is in styleMap map.
        if (!GetColorForStyle(value, m_color))
        {
          // Remove leading '#' symbol.
          std::string styleId = m_mapStyle2Style[value.substr(1)];
          if (!styleId.empty())
            GetColorForStyle(styleId, m_color);
        }
      }
      else if (currTag == "description")
      {
        m_description = value;
      }
    }
    else if (prevTag == "LineStyle" && currTag == "color")
    {
      ParseColor(value);
    }
    else if (ppTag == kStyleMap && prevTag == kPair && currTag == kStyleUrl &&
             m_styleUrlKey == "normal")
    {
      if (!m_mapStyleId.empty())
        m_mapStyle2Style[m_mapStyleId] = value;
    }
    else if (ppTag == kStyleMap && prevTag == kPair && currTag == "key")
    {
      m_styleUrlKey = value;
    }
    else if (ppTag == kPlacemark)
    {
      if (prevTag == "Point")
      {
        if (currTag == "coordinates")
          SetOrigin(value);
      }
      else if (prevTag == "LineString")
      {
        if (currTag == "coordinates")
          ParseLineCoordinates(value, " \n\r\t", ",");
      }
      else if (prevTag == "gx:Track")
      {
        if (currTag == "gx:coord")
          ParseLineCoordinates(value, "\n\r\t", " ");
      }
      else if (prevTag == "ExtendedData")
      {
        if (currTag == "mwm:scale")
        {
          double scale;
          if (!strings::to_double(value, scale))
            scale = 0.0;
          m_viewportScale = static_cast<uint8_t>(scale);
        }
      }
      else if (prevTag == "TimeStamp")
      {
        if (currTag == "when")
        {
          auto const ts = my::StringToTimestamp(value);
          if (ts != my::INVALID_TIME_STAMP)
            m_timestamp = std::chrono::system_clock::from_time_t(ts);
        }
      }
      else if (currTag == kStyleUrl)
      {
        GetColorForStyle(value, m_color);
      }
    }
    else if (ppTag == "MultiGeometry")
    {
      if (prevTag == "Point")
      {
        if (currTag == "coordinates")
          SetOrigin(value);
      }
      else if (prevTag == "LineString")
      {
        if (currTag == "coordinates")
          ParseLineCoordinates(value, " \n\r\t", ",");
      }
      else if (prevTag == "gx:Track")
      {
        if (currTag == "gx:coord")
          ParseLineCoordinates(value, "\n\r\t", " ");
      }
    }
    else if (ppTag == "gx:MultiTrack")
    {
      if (prevTag == "gx:Track")
      {
        if (currTag == "gx:coord")
          ParseLineCoordinates(value, "\n\r\t", " ");
      }
    }
  }
}

}  // namespace kml
