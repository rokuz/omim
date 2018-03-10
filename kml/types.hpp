#pragma once

#include "kml/type_utils.hpp"

#include "base/visitor.hpp"

#include <cmath>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace kml
{
enum class PredefinedColor : uint8_t
{
  None = 0,
  Red,
  Blue,
  Purple,
  Yellow,
  Pink,
  Brown,
  Green,
  Orange
};

inline std::string DebugPrint(PredefinedColor color)
{
  switch (color)
  {
    case PredefinedColor::None: return "None";
    case PredefinedColor::Red: return "Red";
    case PredefinedColor::Blue: return "Blue";
    case PredefinedColor::Purple: return "Purple";
    case PredefinedColor::Yellow: return "Yellow";
    case PredefinedColor::Pink: return "Pink";
    case PredefinedColor::Brown: return "Brown";
    case PredefinedColor::Green: return "Green";
    case PredefinedColor::Orange: return "Orange";
  }
}

struct ColorData
{
  DECLARE_VISITOR(visitor(m_predefinedColor, "predefinedColor"),
                  visitor(m_rgba, "rgba"))

  bool operator == (ColorData const & data) const
  {
    return m_predefinedColor == data.m_predefinedColor && m_rgba == data.m_rgba;
  }

  bool operator != (ColorData const & data) const
  {
    return !operator==(data);
  }

  // Predefined color.
  PredefinedColor m_predefinedColor = PredefinedColor::None;
  // Color in RGBA format.
  uint32_t m_rgba = 0;
};

inline std::string DebugPrint(ColorData const & data)
{
  std::ostringstream os;
  os << "ColorData [";
  os << "predefinedColor:" << DebugPrint(data.m_predefinedColor) << ", ";
  os << "rgba:" << std::hex << data.m_rgba << "]";
  return os.str();
}

struct BookmarkData
{
  DECLARE_VISITOR(visitor(m_id, "id"),
                  visitor(m_name, "name"),
                  visitor(m_description, "description"),
                  visitor(m_color, "color"),
                  visitor(m_viewportScale, "viewportScale"),
                  visitor(m_timestamp, "timestamp"),
                  visitor(m_point, "point"),
                  VISITOR_COLLECTABLE)

  DECLARE_COLLECTABLE(LocalizableStringIndex, m_name, m_description)

  bool operator == (BookmarkData const & data) const
  {
    double constexpr kEps = 1e-5;
    return m_id == data.m_id && m_name == data.m_name &&
           m_description == data.m_description && m_color == data.m_color &&
           m_viewportScale == data.m_viewportScale &&
           Compare(m_timestamp, data.m_timestamp) && m_point.EqualDxDy(data.m_point, kEps);
  }

  bool operator != (BookmarkData const & data) const
  {
    return !operator==(data);
  }

  // Unique id.
  uint64_t m_id = 0;
  // Bookmark's name.
  LocalizableString m_name;
  // Bookmark's description.
  LocalizableString m_description;
  // Bookmark's color.
  ColorData m_color;
  // Viewport scale. 0 is a default value (no scale set).
  uint8_t m_viewportScale = 0;
  // Creation timestamp.
  Timestamp m_timestamp = {};
  // Coordinates in mercator.
  m2::PointD m_point;
};

inline std::string DebugPrint(BookmarkData const & data)
{
  std::ostringstream os;
  os << "BookmarkData [ ";
  os << "id:" << data.m_id << ", ";
  os << "name:" << DebugPrint(data.m_name) << ", ";
  os << "description:" << DebugPrint(data.m_description) << ", ";
  os << "color:" << DebugPrint(data.m_color) << ", ";
  os << "viewportScale:" << static_cast<uint32_t>(data.m_viewportScale) << ", ";
  os << "timestamp:" << DebugPrint(data.m_timestamp) << ", ";
  os << "point:" << DebugPrint(data.m_point) << " ]";
  return os.str();
}

struct TrackLayer
{
  DECLARE_VISITOR(visitor(m_lineWidth, "lineWidth"),
                  visitor(m_color, "color"))

  bool operator == (TrackLayer const & layer) const
  {
    double constexpr kEps = 1e-5;
    return m_color == layer.m_color && fabs(m_lineWidth - layer.m_lineWidth) < kEps;
  }

  bool operator != (TrackLayer const & layer) const
  {
    return !operator==(layer);
  }

  // Line width in pixels. Valid range is [kMinLineWidth; kMaxLineWidth].
  double m_lineWidth = 5.0;
  // Layer's color.
  ColorData m_color;
};

inline std::string DebugPrint(TrackLayer const & layer)
{
  std::ostringstream os;
  os << "TrackLayer [";
  os << "lineWidth:" << layer.m_lineWidth << ", ";
  os << "color:" << DebugPrint(layer.m_color) << "]";
  return os.str();
}

struct TrackData
{
  DECLARE_VISITOR(visitor(m_id, "id"),
                  visitor(m_name, "name"),
                  visitor(m_description, "description"),
                  visitor(m_layers, "layers"),
                  visitor(m_timestamp, "timestamp"),
                  visitor(m_points, "points"),
                  VISITOR_COLLECTABLE)

  DECLARE_COLLECTABLE(LocalizableStringIndex, m_name, m_description)

  bool operator == (TrackData const & data) const
  {
    return m_id == data.m_id && m_name == data.m_name &&
           m_description == data.m_description && m_layers == data.m_layers &&
           Compare(m_timestamp, data.m_timestamp) && Compare(m_points, data.m_points);
  }

  bool operator != (TrackData const & data) const
  {
    return !operator==(data);
  }

  // Unique id.
  uint64_t m_id = 0;
  // Track's name.
  LocalizableString m_name;
  // Track's description.
  LocalizableString m_description;
  // Layers.
  std::vector<TrackLayer> m_layers;
  // Creation timestamp.
  Timestamp m_timestamp = {};
  // Points.
  std::vector<m2::PointD> m_points;
};

inline std::string DebugPrint(TrackData const & data)
{
  std::ostringstream os;
  os << "TrackData [ ";
  os << "id:" << data.m_id << ", ";
  os << "name:" << DebugPrint(data.m_name) << ", ";
  os << "description:" << DebugPrint(data.m_description) << ", ";
  os << "layers:" << ::DebugPrint(data.m_layers) << ", ";
  os << "timestamp:" << DebugPrint(data.m_timestamp) << ", ";
  os << "points:" << ::DebugPrint(data.m_points) << " ]";
  return os.str();
}

struct CategoryData
{
  DECLARE_VISITOR(visitor(m_name, "name"),
                  visitor(m_bookmarksData, "bookmarks"),
                  visitor(m_tracksData, "tracks"),
                  visitor(m_visible, "visible"),
                  visitor(m_public, "public"),
                  VISITOR_COLLECTABLE)

  DECLARE_COLLECTABLE(LocalizableStringIndex, m_name)

  bool operator == (CategoryData const & data) const
  {
    return m_name == data.m_name && m_visible == data.m_visible &&
           m_public == data.m_public && Compare(m_bookmarksData, data.m_bookmarksData) &&
           Compare(m_tracksData, data.m_tracksData);
  }

  bool operator != (CategoryData const & data) const
  {
    return !operator==(data);
  }

  LocalizableString m_name;
  std::vector<std::shared_ptr<BookmarkData>> m_bookmarksData;
  std::vector<std::shared_ptr<TrackData>> m_tracksData;
  bool m_visible = true;
  bool m_public = false;
};

inline std::string DebugPrint(CategoryData const & data)
{
  std::ostringstream os;
  os << "CategoryData [ ";
  os << "name:" << DebugPrint(data.m_name) << ", ";
  os << "visible:" << data.m_visible << ", ";
  os << "public:" << data.m_public << ", ";
  os << "bookmarks:" << DebugPrint(data.m_bookmarksData) << ", ";
  os << "tracks:" << DebugPrint(data.m_tracksData) << " ]";
  return os.str();
}
}  // namespace kml
