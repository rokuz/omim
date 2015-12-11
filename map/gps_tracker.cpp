#include "map/gps_tracker.hpp"
#include "map/framework.hpp"

#include "coding/file_name_utils.hpp"

#include "platform/platform.hpp"

#include "std/atomic.hpp"

#include "defines.hpp"

namespace
{

char const kEnabledKey[] = "GpsTrackingEnabled";
char const kDurationHours[] = "GpsTrackingDuration";
uint32_t constexpr kDefaultDurationHours = 24;

size_t constexpr kMaxItemCount = 100000; // > 24h with 1point/s

inline string GetFilePath()
{
  return my::JoinFoldersToPath(GetPlatform().WritableDir(), GPS_TRACK_FILENAME);
}

inline bool GetSettingsIsEnabled()
{
  bool enabled = false;
  Settings::Get(kEnabledKey, enabled);
  return enabled;
}

inline void SetSettingsIsEnabled(bool enabled)
{
  Settings::Set(kEnabledKey, enabled);
}

inline hours GetSettingsDuration()
{
  uint32_t duration = kDefaultDurationHours;
  Settings::Get(kDurationHours, duration);
  return hours(duration);
}

inline void SetSettingsDuration(hours duration)
{
  uint32_t const hours = duration.count();
  Settings::Set(kDurationHours, hours);
}

} // namespace

GpsTracker & GpsTracker::Instance()
{
  static GpsTracker instance;
  return instance;
}

GpsTracker::GpsTracker()
  : m_enabled(GetSettingsIsEnabled())
  , m_duration(GetSettingsDuration())
  , m_track(GetFilePath(), kMaxItemCount, m_duration)
{
}

void GpsTracker::SetEnabled(bool enabled)
{  
  if (enabled == m_enabled)
    return;

  SetSettingsIsEnabled(enabled);
  m_enabled = enabled;

  if (enabled)
    m_track.Clear();
}

bool GpsTracker::IsEnabled() const
{
  return m_enabled;
}

void GpsTracker::SetDuration(hours duration)
{
  SetSettingsDuration(duration);
  m_duration = duration;
}

hours GpsTracker::GetDuration() const
{
  return m_duration;
}

void GpsTracker::Connect(TGpsTrackDiffCallback const & fn)
{
  m_track.SetCallback(fn);
}

void GpsTracker::Disconnect()
{
  m_track.SetCallback(nullptr);
}

void GpsTracker::OnLocationUpdated(location::GpsInfo const & info)
{
  if (!m_enabled)
    return;
  m_track.AddPoint(info);
}
