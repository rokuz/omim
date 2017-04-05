#pragma once

#include "geometry/rect2d.hpp"

#include "indexer/index.hpp"
#include "indexer/mwm_set.hpp"

#include "base/thread.hpp"

#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>

class LocalAdsStatistics final
{
public:
  using Timestamp = std::chrono::steady_clock::time_point;

  enum class EventType
  {
    ShowPoint = 0,
    OpenInfo
  };

  struct Event
  {
    EventType m_type;
    uint32_t m_mwmVersion;
    std::string m_countryId;
    uint32_t m_featureId;
    uint8_t m_zoomLevel;
    Timestamp m_timestamp;

    Event(EventType type, uint32_t mwmVersion, std::string const & countryId,
          uint32_t featureId, uint8_t zoomLevel, Timestamp const & timestamp)
      : m_type(type)
      , m_mwmVersion(mwmVersion)
      , m_countryId(countryId)
      , m_featureId(featureId)
      , m_zoomLevel(zoomLevel)
      , m_timestamp(timestamp)
    {}

    bool operator < (Event const & event) const
    {
      if (m_mwmVersion == event.m_mwmVersion)
      {
        if (m_countryId == event.m_countryId)
          return m_timestamp < event.m_timestamp;
        return m_countryId < event.m_countryId;
      }
      return m_mwmVersion < event.m_mwmVersion;
    }

    bool operator == (Event const & event) const
    {
      using namespace std::chrono;
      return m_type == event.m_type && m_mwmVersion == event.m_mwmVersion &&
             m_countryId == event.m_countryId && m_featureId == event.m_featureId &&
             m_zoomLevel == event.m_zoomLevel &&
             duration_cast<seconds>(m_timestamp - event.m_timestamp).count() == 0;
    }
  };

  struct PackedData
  {
    // 32 bit: feature id.
    // 24 bit: seconds (about half of year).
    // 5 bit:  zoom level [1-19].
    // 3 bit:  event type.
    uint64_t m_featureIndex : 32, m_seconds : 24, m_zoomLevel : 5, m_eventType : 3;
  };

  LocalAdsStatistics();
  ~LocalAdsStatistics();

  void Teardown();

  void RegisterEvent(Event && event);
  void RegisterEvents(std::list<Event> && events);

  std::list<Event> WriteEventsForTesting(std::list<Event> & events);
  std::list<Event> ReadEventsForTesting(std::string const & fileName,
                                        Timestamp const & minTimestamp);
  void CleanupAfterTesting();

private:
  void ThreadRoutine();
  bool RequestEvents(std::list<Event> & events, bool & needToSend);

  void IndexMetadata();
  void ExtractMetadata(std::string const & fileName);
  std::list<Event> WriteEvents(std::list<Event> & events, std::string & fileName);
  std::list<Event> ReadEvents(std::string const & fileName, Timestamp const & minTimestamp);

  using MetadataKey = std::pair<std::string, uint32_t>;
  struct Metadata
  {
    std::string m_fileName;
    Timestamp m_timestamp;

    Metadata() = default;
    Metadata(std::string const & fileName, Timestamp const & timestamp)
      : m_fileName(fileName), m_timestamp(timestamp)
    {}
  };
  std::map<MetadataKey, Metadata> m_metadataCache;
  Timestamp m_lastSending;

  bool m_isRunning = true;
  std::condition_variable m_condition;
  std::list<Event> m_events;
  std::mutex m_mutex;
  threads::SimpleThread m_thread;
};

extern string DebugPrint(LocalAdsStatistics::Event const & event);
