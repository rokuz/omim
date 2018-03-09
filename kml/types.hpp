#pragma once

#include "geometry/point2d.hpp"

#include "base/visitor.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

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

using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
using LocalizableString = std::unordered_map<uint8_t, std::string>;

struct ColorData
{
  // Predefined color.
  PredefinedColor m_predefinedColor = PredefinedColor::None;
  // Color in RGBA format.
  uint32_t m_rgba = 0;
};

struct BookmarkData
{
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

struct TrackLayer
{
  // Line width in pixels.
  float m_lineWidth = 5.0f;
  // Layer's color.
  ColorData m_color;
};

struct TrackData
{
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

struct CategoryData
{
  LocalizableString m_name;
  std::vector<std::shared_ptr<BookmarkData>> m_bookmarksData;
  std::vector<std::shared_ptr<TrackData>> m_tracksData;
  bool m_visible = true;
  bool m_public = false;
};
}  // namespace kml
