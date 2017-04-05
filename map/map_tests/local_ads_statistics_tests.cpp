#include "testing/testing.hpp"

#include "map/local_ads_statistics.hpp"

namespace
{
template<typename Duration>
LocalAdsStatistics::Timestamp TimestampInPast(LocalAdsStatistics::Timestamp baseTimestamp,
                                              Duration const &duration)
{
  return baseTimestamp - duration;
}

class StatisticsGuard
{
public:
  StatisticsGuard(LocalAdsStatistics & statistics)
    : m_statistics(statistics)
  {}

  ~StatisticsGuard()
  {
    m_statistics.Teardown();
    m_statistics.CleanupAfterTesting();
  }

private:
  LocalAdsStatistics & m_statistics;
};

}

UNIT_TEST(LocalAdsStatistics_Read_Write_Simple)
{
  using namespace std::chrono;
  using ET = LocalAdsStatistics::EventType;

  LocalAdsStatistics::Timestamp baseTs = std::chrono::steady_clock::now();

  LocalAdsStatistics statistics;
  StatisticsGuard guard(statistics);

  std::list<LocalAdsStatistics::Event> events;
  events.emplace_back(ET::ShowPoint, 123456, "Moscow", 111, 15, TimestampInPast(baseTs, minutes(15)));
  events.emplace_back(ET::ShowPoint, 123456, "Moscow", 222, 13, TimestampInPast(baseTs, minutes(10)));
  events.emplace_back(ET::OpenInfo, 123456, "Moscow", 111, 17, TimestampInPast(baseTs, minutes(5)));
  auto unprocessedEvents = statistics.WriteEventsForTesting(events);
  TEST_EQUAL(unprocessedEvents.size(), 0, ());

  auto result = statistics.ReadEventsForTesting("Moscow_123456.dat", TimestampInPast(baseTs, minutes(60)));
  TEST_EQUAL(events, result, ());
}
