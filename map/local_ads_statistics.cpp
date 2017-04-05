#include "map/local_ads_statistics.hpp"

#include "drape_frontend/drape_engine.hpp"
#include "drape_frontend/visual_params.hpp"

#include "indexer/scales.hpp"

#include "platform/http_client.hpp"
#include "platform/platform.hpp"

#include "coding/file_container.hpp"
#include "coding/file_name_utils.hpp"
#include "coding/file_writer.hpp"

#include "base/bits.hpp"
#include "base/string_utils.hpp"

#include <sstream>
#include <functional>

namespace
{
std::string const kStatisticsFolder = "local_ads_stats";
std::string const kStatisticsExt = ".dat";

std::string const kStatisticsFile = "local_ads_stats.dat";
//size_t constexpr kMaxFileSizeInBytes = 10 * 1024 * 1024;
auto constexpr kSendingTimeout = std::chrono::hours(1);
int64_t constexpr kEventMaxLifetimeInSeconds = 24 * 183 * 3600; // About half of year.
auto constexpr kDeletionPeriod = std::chrono::hours(24);

void WriteMetadata(FileWriter & writer, std::string const & countryId, uint32_t mwmVersion,
                   LocalAdsStatistics::Timestamp const & ts)
{
  ASSERT(!countryId.empty(), ());
  ASSERT_LESS(countryId.length(), (1 << sizeof(uint8_t)) * 8 - 1, ());
  uint8_t len = static_cast<uint8_t>(countryId.length() + 1);
  WriteToSink(writer, len);
  writer.Write(countryId.c_str(), len);

  WriteToSink(writer, mwmVersion);

  int64_t const seconds = duration_cast<std::chrono::seconds>(ts.time_since_epoch()).count();
  uint64_t encodedSeconds = bits::ZigZagEncode(seconds);
  WriteToSink(writer, encodedSeconds);
}

template<typename TPrimitive>
TPrimitive ReadPrimitive(FileReader const & reader, uint64_t & offset)
{
  std::vector<char> bytes(sizeof(TPrimitive));
  ReadFromPos(reader, offset, bytes.data(), bytes.size());
  offset += bytes.size();

  MemReaderWithExceptions memReader(bytes.data(), bytes.size());
  ReaderSource<decltype(memReader)> src(memReader);
  return ReadPrimitiveFromSource<uint32_t>(src);
}

uint64_t ReadMetadata(FileReader const & reader, std::string & countryId,
                      uint32_t & mwmVersion, LocalAdsStatistics::Timestamp & ts)
{
  uint64_t offset = 0;
  uint8_t countryIdLen = 0;
  ReadFromPos(reader, offset, &countryIdLen, sizeof(countryIdLen));
  offset += sizeof(countryIdLen);
  ASSERT_NOT_EQUAL(countryIdLen, 0, ());

  std::vector<char> bytes(countryIdLen);
  ReadFromPos(reader, offset, bytes.data(), bytes.size());
  offset += bytes.size();
  countryId = static_cast<char const *>(bytes.data());

  mwmVersion = ReadPrimitive<uint32_t>(reader, offset);

  uint64_t const seconds = ReadPrimitive<uint64_t>(reader, offset);
  int64_t const decodedSeconds = bits::ZigZagDecode(seconds);
  ts = LocalAdsStatistics::Timestamp(std::chrono::seconds(decodedSeconds));

  return offset;
}

void WritePackedData(FileWriter & writer, LocalAdsStatistics::PackedData & packedData)
{
  // PackedData is bit field (uint64_t).
  ASSERT_EQUAL(sizeof(packedData), sizeof(uint64_t), ());
  uint64_t * val = reinterpret_cast<uint64_t *>(&packedData);
  WriteToSink(writer, *val);
}

using ReadCallback = std::function<void(LocalAdsStatistics::PackedData &&,
                                        std::string const &, uint32_t,
                                        LocalAdsStatistics::Timestamp const &)>;

void ReadPackedData(FileReader const & reader, ReadCallback const & callback)
{
  if (callback == nullptr)
    return;

  using PackedData = LocalAdsStatistics::PackedData;

  std::string countryId;
  uint32_t mwmVersion;
  LocalAdsStatistics::Timestamp baseTimestamp;
  auto metadataSize = ReadMetadata(reader, countryId, mwmVersion, baseTimestamp);

  auto restSize = reader.Size() - metadataSize;
  if (restSize == 0)
    return;
  ASSERT(restSize % sizeof(PackedData) == 0, ());

  std::vector<uint8_t> bytes(restSize);
  ReadFromPos(reader, metadataSize, bytes.data(), bytes.size());
  MemReaderWithExceptions memReader(bytes.data(), bytes.size());
  for (uint64_t i = 0; i < static_cast<uint64_t>(restSize) / sizeof(PackedData); i++)
  {
    callback(ReadPrimitiveFromPos<PackedData>(memReader, i * sizeof(PackedData)),
             countryId, mwmVersion, baseTimestamp);
  }
}

LocalAdsStatistics::Timestamp GetMinTimestamp(std::list<LocalAdsStatistics::Event> const & events,
                                              std::string const & countryId,
                                              LocalAdsStatistics::Timestamp const & defTimestamp)
{
  LocalAdsStatistics::Timestamp minTimestamp = defTimestamp;
  for (auto const & event : events)
  {
    if (event.m_countryId != countryId)
      continue;

    if (event.m_timestamp < minTimestamp)
      minTimestamp = event.m_timestamp;
  }
  return minTimestamp;
}

std::string GetPath(std::string const & fileName)
{
  return my::JoinFoldersToPath({GetPlatform().WritableDir(), kStatisticsFolder}, fileName);
}

std::string GetPath(LocalAdsStatistics::Event const & event)
{
  return GetPath(event.m_countryId + "_" + strings::to_string(event.m_mwmVersion) + kStatisticsExt);
}

void CreateDirIfNotExist()
{
  std::string const statsFolder = GetPath("");
  if (!GetPlatform().IsFileExistsByFullPath(statsFolder))
    GetPlatform().MkDir(statsFolder);
}
}  // namespace

LocalAdsStatistics::LocalAdsStatistics()
  : m_thread(&LocalAdsStatistics::ThreadRoutine, this)
{}

LocalAdsStatistics::~LocalAdsStatistics()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ASSERT(!m_isRunning, ());
}

void LocalAdsStatistics::Teardown()
{
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isRunning)
      return;
    m_isRunning = false;
  }
  m_condition.notify_one();
  m_thread.join();
}

bool LocalAdsStatistics::RequestEvents(std::list<Event> & events, bool & needToSend)
{
  unique_lock<mutex> lock(m_mutex);

  needToSend = !m_condition.wait_for(lock, kSendingTimeout, [this]
  {
    return !m_isRunning || !m_events.empty();
  });

  needToSend |= steady_clock::now() > (m_lastSending + kSendingTimeout);

  if (!m_isRunning)
    return false;

  if (!m_events.empty())
    events.swap(m_events);

  return true;
}

void LocalAdsStatistics::RegisterEvent(Event && event)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_events.push_back(std::move(event));
}

void LocalAdsStatistics::RegisterEvents(std::list<Event> && events)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_events.splice(m_events.end(), std::move(events));
}

void LocalAdsStatistics::ThreadRoutine()
{
  std::list<Event> events;
  bool needToSend = false;
  while (RequestEvents(events, needToSend))
  {
    bool needRebuild;
    do
    {
      std::string fileNameToRebuild;
      auto unprocessedEvents = WriteEvents(events, fileNameToRebuild);
      needRebuild = !unprocessedEvents.empty();

      if (needRebuild)
      {
        // The first event in the list is cause of writing interruption.
        Event const & event = unprocessedEvents.front();
        auto constexpr kLifetime = std::chrono::seconds(kEventMaxLifetimeInSeconds);
        auto minTimestamp = event.m_timestamp - kLifetime + kDeletionPeriod;

        m_metadataCache.erase(MetadataKey(event.m_countryId, event.m_mwmVersion));
        std::list<Event> newEvents = ReadEvents(fileNameToRebuild, minTimestamp);
        newEvents.splice(newEvents.end(), std::move(unprocessedEvents));
        FileWriter::DeleteFileX(fileNameToRebuild);
        std::swap(events, newEvents);
      }
    }
    while (needRebuild);
    events.clear();

    // Send statistics to server.
    if (needToSend)
    {
      //TODO: implement
      //TODO: delete file in success
    }
  }
}

std::list<LocalAdsStatistics::Event> LocalAdsStatistics::WriteEvents(std::list<Event> & events,
                                                                     std::string & fileName)
{
  try
  {
    CreateDirIfNotExist();
    if (m_metadataCache.empty())
      IndexMetadata();

    std::unique_ptr<FileWriter> writer;

    events.sort();

    auto eventIt = events.begin();
    for (; eventIt != events.end(); ++eventIt)
    {
      Event const & event = *eventIt;
      MetadataKey const key = std::make_pair(event.m_countryId, event.m_mwmVersion);
      auto it = m_metadataCache.find(key);

      // Get metadata.
      Metadata metadata;
      bool needWriteMetadata = false;
      if (it == m_metadataCache.end())
      {
        metadata.m_timestamp = GetMinTimestamp(events, event.m_countryId, event.m_timestamp);
        metadata.m_fileName = GetPath(event);
        m_metadataCache[key] = metadata;
        needWriteMetadata = true;
      }
      else
      {
        metadata = m_metadataCache[key];
      }

      if (writer == nullptr || writer->GetName() != metadata.m_fileName)
        writer = my::make_unique<FileWriter>(metadata.m_fileName, FileWriter::OP_APPEND);

      if (needWriteMetadata)
        WriteMetadata(*writer, event.m_countryId, event.m_mwmVersion, metadata.m_timestamp);

      // Check if timestamp is out of date. In this case we have to rebuild events package.
      using namespace std::chrono;
      int64_t const s = duration_cast<seconds>(event.m_timestamp - metadata.m_timestamp).count();
      if (s < 0 || s > kEventMaxLifetimeInSeconds)
      {
        // Return unprocessed events.
        std::list<Event> unprocessedEvents;
        unprocessedEvents.splice(unprocessedEvents.end(), events, eventIt, events.end());
        return unprocessedEvents;
      }

      PackedData data;
      data.m_featureIndex = event.m_featureId;
      data.m_seconds = static_cast<uint32_t>(s);
      data.m_zoomLevel = event.m_zoomLevel;
      data.m_eventType = static_cast<uint8_t>(event.m_type);
      WritePackedData(*writer, data);
    }
  }
  catch (RootException const & ex)
  {
    LOG(LWARNING, (ex.Msg()));
  }
  return std::list<Event>();
}

std::list<LocalAdsStatistics::Event> LocalAdsStatistics::ReadEvents(std::string const & fileName,
                                                                    Timestamp const & minTimestamp)
{
  std::list<Event> result;
  if (!GetPlatform().IsFileExistsByFullPath(fileName))
    return result;

  try
  {
    FileReader reader(fileName);
    ReadPackedData(reader, [&minTimestamp, &result](PackedData && data, std::string const & countryId,
                                                    uint32_t mwmVersion, Timestamp const & baseTimestamp)
    {
      auto ts = baseTimestamp + std::chrono::seconds(data.m_seconds);
      if (ts <= minTimestamp)
        return;
      result.emplace_back(static_cast<EventType>(data.m_eventType), mwmVersion,
                          countryId, static_cast<uint32_t>(data.m_featureIndex),
                          static_cast<uint8_t>(data.m_zoomLevel), ts);
    });
  }
  catch (Reader::Exception const & ex)
  {
    LOG(LWARNING, ("Error reading file:", fileName, ex.Msg()));
  }
  return result;
}

std::list<LocalAdsStatistics::Event> LocalAdsStatistics::WriteEventsForTesting(std::list<Event> & events)
{
  std::list<Event> mutableEvents = events;
  std::string unusedFileName;
  return WriteEvents(mutableEvents, unusedFileName);
}

std::list<LocalAdsStatistics::Event> LocalAdsStatistics::ReadEventsForTesting(std::string const & fileName,
                                                                              Timestamp const & minTimestamp)
{
  return ReadEvents(GetPath(fileName), minTimestamp);
}

void LocalAdsStatistics::CleanupAfterTesting()
{
  std::string const statsFolder = GetPath("");
  if (GetPlatform().IsFileExistsByFullPath(statsFolder))
    GetPlatform().RmDirRecursively(statsFolder);
}

void LocalAdsStatistics::IndexMetadata()
{
  std::string const statsFolder = GetPath("");
  std::vector<std::string> files;
  GetPlatform().GetFilesByExt(statsFolder, kStatisticsExt, files);
  for (auto const & filename : files)
    ExtractMetadata(GetPath(filename));
}
void LocalAdsStatistics::ExtractMetadata(std::string const & fileName)
{
  ASSERT(GetPlatform().IsFileExistsByFullPath(fileName), ());
  try
  {
    FileReader reader(fileName);
    std::string countryId;
    uint32_t mwmVersion;
    LocalAdsStatistics::Timestamp baseTimestamp;
    ReadMetadata(reader, countryId, mwmVersion, baseTimestamp);
    MetadataKey const key = std::make_pair(countryId, mwmVersion);

    auto it = m_metadataCache.find(key);
    if (it == m_metadataCache.end() || it->second.m_timestamp < baseTimestamp)
      m_metadataCache[key] = Metadata(fileName, baseTimestamp);
  }
  catch (Reader::Exception const & ex)
  {
    LOG(LWARNING, ("Error reading file:", fileName, ex.Msg()));
  }
}

string DebugPrint(LocalAdsStatistics::Event const & event)
{
  std::stringstream s;
  s << "[Type:" << static_cast<uint32_t>(event.m_type) << "; Country: " << event.m_countryId
    << "; FID: " << event.m_featureId << "; Zoom: " << static_cast<uint32_t>(event.m_zoomLevel)
    << "; Ts: " << duration_cast<std::chrono::seconds>(event.m_timestamp.time_since_epoch()).count() << "]";
  return s.str();
}

/*string LocalAdsStatistics::MakeRemoteURL(MwmSet::MwmId const &mwmId) const
{
  // TODO: build correct URL after server completion.

  return "http://172.27.15.68/campaigns.data";
}
}*/
