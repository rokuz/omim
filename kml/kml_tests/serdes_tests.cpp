#include "testing/testing.hpp"

#include "kml/serdes.hpp"
#include "kml/serdes_binary.hpp"

#include "coding/file_name_utils.hpp"
#include "coding/file_reader.hpp"
#include "coding/file_writer.hpp"
#include "coding/reader.hpp"
#include "coding/writer.hpp"
#include "coding/zip_reader.hpp"

#include "base/scope_guard.hpp"

#include <chrono>
#include <cstring>
#include <memory>
#include <vector>

namespace
{
char const * kKmlSrc =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<kml xmlns=\"http://earth.google.com/kml/2.2\">"
    "<Document>"
      "<name>MapName</name>"
      "<description><![CDATA[MapDescription]]></description>"
      "<visibility>0</visibility>"
      "<Style id=\"placemark-blue\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-blue.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Style id=\"placemark-brown\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-brown.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Style id=\"placemark-green\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-green.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Style id=\"placemark-orange\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-orange.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Style id=\"placemark-pink\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-pink.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Style id=\"placemark-purple\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-purple.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Style id=\"placemark-red\">"
        "<IconStyle>"
          "<Icon>"
            "<href>http://www.mapswithme.com/placemarks/placemark-red.png</href>"
          "</Icon>"
        "</IconStyle>"
      "</Style>"
      "<Placemark>"
        "<name>Nebraska</name>"
        "<description><![CDATA[]]></description>"
        "<styleUrl>#placemark-red</styleUrl>"
        "<Point>"
          "<coordinates>-99.901810,41.492538,0.000000</coordinates>"
        "</Point>"
      "</Placemark>"
      "<Placemark>"
        "<name>Monongahela National Forest</name>"
        "<description><![CDATA[Huttonsville, WV 26273<br>]]></description>"
        "<styleUrl>#placemark-pink</styleUrl>"
        "<TimeStamp>"
          "<when>1986-08-12T07:10:43Z</when>"
        "</TimeStamp>"
        "<Point>"
          "<coordinates>-79.829674,38.627785,0.000000</coordinates>"
        "</Point>"
      "</Placemark>"
      "<Placemark>"
        "<name>From: Минск, Минская область, Беларусь</name>"
        "<description><![CDATA[]]></description>"
        "<styleUrl>#placemark-blue</styleUrl>"
        "<TimeStamp>"
          "<when>1998-03-03T03:04:48+01:30</when>"
        "</TimeStamp>"
        "<Point>"
          "<coordinates>27.566765,53.900047,0</coordinates>"
        "</Point>"
      "</Placemark>"
      "<Placemark>"
        "<name><![CDATA[<MWM & Sons>]]></name>"
        "<description><![CDATA[Amps & <brackets>]]></description>"
        "<styleUrl>#placemark-green</styleUrl>"
        "<TimeStamp>"
          "<when>2048 bytes in two kilobytes - some invalid timestamp</when>"
        "</TimeStamp>"
        "<Point>"
          "<coordinates>27.551532,53.89306</coordinates>"
        "</Point>"
      "</Placemark>"
    "</Document>"
    "</kml>";
}  // namespace

using KmlMemoryDeserializer = kml::DeserializerKml<MemReader>;
using KmlFileDeserializer = kml::DeserializerKml<FileReader>;

UNIT_TEST(Kml_Deserialization)
{
  kml::CategoryData data;
  try
  {
    KmlMemoryDeserializer des(kKmlSrc, strlen(kKmlSrc));
    des(data);
  }
  catch (KmlMemoryDeserializer::DeserializeException &exc)
  {
    TEST(false, ("Exception raised", exc.what()));
  }

  std::vector<uint8_t> buffer;
  {
    using Sink = MemWriter<decltype(buffer)>;
    Sink sink(buffer);
    kml::binary::SerializerKml ser(data, {});
    ser.Serialize(sink);
  }

  kml::CategoryData data2;
  {
    MemReader reader(buffer.data(), buffer.size());
    kml::binary::DeserializerKml des(data2, {});
    des.Deserialize(reader);
  }

  TEST_EQUAL(data, data2, ());
}

/*UNIT_TEST(Kml_File_Deserialization)
{
  kml::CategoryData data;
  try
  {
    KmlFileDeserializer des("/Users/romankuznetsov/Dev/Projects/omim/data/world.kml");
    des(data);
  }
  catch (KmlMemoryDeserializer::DeserializeException &exc)
  {
    TEST(false, ("Exception raised", exc.what()));
  }

  std::vector<uint8_t> buffer;
  {
    using Sink = MemWriter<decltype(buffer)>;
    Sink sink(buffer);
    kml::binary::SerializerKml ser(data, {});
    ser.Serialize(sink);
  }

  {
    FileWriter file("/Users/romankuznetsov/Dev/Projects/omim/data/world.kmb");
    file.Write(buffer.data(), buffer.size());
  }

  kml::CategoryData data2;
  {
    MemReader reader(buffer.data(), buffer.size());
    kml::binary::DeserializerKml des(data2, {});
    des.Deserialize(reader);
  }

  TEST_EQUAL(data, data2, ());
}

UNIT_TEST(Kml_Speed_Text)
{
  kml::CategoryData data;
  try
  {
    KmlFileDeserializer des("/Users/romankuznetsov/Dev/Projects/omim/data/world.kml");
    des(data);
  }
  catch (KmlMemoryDeserializer::DeserializeException &exc)
  {
    TEST(false, ("Exception raised", exc.what()));
  }
}

UNIT_TEST(Kml_Speed_Bin)
{
  kml::CategoryData data;
  FileReader reader("/Users/romankuznetsov/Dev/Projects/omim/data/world.kmb");
  kml::binary::DeserializerKml des(data, {});
  des.Deserialize(reader);
}*/

/*UNIT_TEST(Kmb_Generator)
{
  std::string const dir = "/Users/romankuznetsov/Dev/Projects/omim/data/kml";
  Platform::FilesList files;
  Platform::GetFilesByExt(dir, ".kml", files);
  for (auto & f : files)
  {
    auto const filePath = dir + "/" + f;
    std::string name = f;
    my::GetNameWithoutExt(name);
    auto const binFilePath = dir + "/" + name + ".kmb";

    kml::CategoryData data;
    try
    {
      KmlFileDeserializer des(filePath);
      des(data);
    }
    catch (KmlFileDeserializer::DeserializeException & exc)
    {
      FileWriter::DeleteFileX(filePath);
      continue;
    }

    std::vector<uint8_t> buffer;
    {
      using Sink = MemWriter<decltype(buffer)>;
      Sink sink(buffer);
      kml::binary::SerializerKml ser(data, {});
      ser.Serialize(sink);
    }

    {
      FileWriter file(binFilePath);
      file.Write(buffer.data(), buffer.size());
    }

    kml::CategoryData data2;
    {
      MemReader reader(buffer.data(), buffer.size());
      kml::binary::DeserializerKml des(data2, {});
      des.Deserialize(reader);
    }

    TEST_EQUAL(data, data2, ());
  }
}*/

struct SpeedTestStat
{
  size_t bookmarksCount = 0;
  long long int d1 = 0;
  long long int d2 = 0;
};

std::pair<double, size_t> CalculateAvgCoef(std::vector<SpeedTestStat> const & stats,
                                           std::function<bool(SpeedTestStat const &)> && filter)
{
  double avg = 0.0;
  size_t cnt = 0;
  for (auto const & s : stats)
  {
    if (filter && !filter(s))
      continue;
    avg += (static_cast<double>(s.d1) / static_cast<double>(s.d2));
    cnt++;
  }
  if (cnt != 0)
    avg /= cnt;
  return std::make_pair(avg, cnt);
}

/*UNIT_TEST(Kmb_Complex_Speed_Test)
{
  std::string const dir = "/Users/romankuznetsov/Dev/Projects/omim/data/kml";
  Platform::FilesList files;
  Platform::GetFilesByExt(dir, ".kml", files);

  std::vector<SpeedTestStat> stats;
  stats.reserve(files.size());

  double avg = 0.0;
  for (auto & f : files)
  {
    auto const filePath = dir + "/" + f;
    std::string name = f;
    my::GetNameWithoutExt(name);
    auto const binFilePath = dir + "/" + name + ".kmb";

    // Text.
    auto ts1 = std::chrono::steady_clock::now();
    {
      kml::CategoryData data;
      try
      {
        KmlFileDeserializer des(filePath);
        des(data);
      }
      catch (KmlFileDeserializer::DeserializeException &exc)
      {
        TEST(false, ("Exception raised", exc.what()));
      }
    }

    // Binary.
    auto ts2 = std::chrono::steady_clock::now();
    kml::CategoryData data;
    {
      FileReader reader(binFilePath);
      kml::binary::DeserializerKml des(data, {});
      des.Deserialize(reader);
    }
    auto ts3 = std::chrono::steady_clock::now();

    auto d1 = std::chrono::duration_cast<std::chrono::nanoseconds>(ts2 - ts1).count();
    auto d2 = std::chrono::duration_cast<std::chrono::nanoseconds>(ts3 - ts2).count();
    avg += (static_cast<double>(d1) / static_cast<double>(d2));

    SpeedTestStat st;
    st.bookmarksCount = data.m_bookmarksData.size() + data.m_tracksData.size();
    st.d1 = d1;
    st.d2 = d2;
    stats.push_back(st);
  }
  avg /= files.size();

  LOG(LINFO, ("Avg speed coef =", avg));

  auto const v1 = CalculateAvgCoef(stats, [](SpeedTestStat const & s) { return s.bookmarksCount < 100; });
  auto const v2 = CalculateAvgCoef(stats, [](SpeedTestStat const & s) { return s.bookmarksCount >= 100 && s.bookmarksCount < 1000; });
  auto const v3 = CalculateAvgCoef(stats, [](SpeedTestStat const & s) { return s.bookmarksCount >= 1000; });

  LOG(LINFO, ("Small: ", v1.second, "Avg speed coef =", v1.first));
  LOG(LINFO, ("Medium: ", v2.second, "Avg speed coef =", v2.first));
  LOG(LINFO, ("Large: ", v3.second, "Avg speed coef =", v3.first));
}*/

UNIT_TEST(Kmb_Complex_Speed_Test_Kmz)
{
  std::string const dir = "/Users/romankuznetsov/Dev/Projects/omim/data/kml";
  Platform::FilesList files;
  Platform::GetFilesByExt(dir, ".kmz", files);

  std::vector<SpeedTestStat> stats;
  stats.reserve(files.size());

  double avg = 0.0;
  for (auto & f : files)
  {
    auto const filePath = dir + "/" + f;
    std::string name = f;
    my::GetNameWithoutExt(name);
    auto const binFilePath = dir + "/" + name + ".kmb";

    // Text.
    auto ts1 = std::chrono::steady_clock::now();
    {
      ZipFileReader::FileListT filesInZip;
      ZipFileReader::FilesList(filePath, filesInZip);

      bool isKMLinZip = false;
      for (size_t i = 0; i < filesInZip.size(); ++i)
      {
        if (filesInZip[i].first == "doc.kml")
        {
          isKMLinZip = true;
          break;
        }
      }
      if (!isKMLinZip)
      {
        FileWriter::DeleteFileX(filePath);
        continue;
      }

      string const kmlFile = dir + "/tmp.kml";
      MY_SCOPE_GUARD(fileGuard, bind(&FileWriter::DeleteFileX, kmlFile));
      ZipFileReader::UnzipFile(filePath, "doc.kml", kmlFile);

      kml::CategoryData data;
      try
      {
        KmlFileDeserializer des(kmlFile);
        des(data);
      }
      catch (KmlFileDeserializer::DeserializeException &exc)
      {
        TEST(false, ("Exception raised", exc.what()));
      }
    }

    // Binary.
    auto ts2 = std::chrono::steady_clock::now();
    kml::CategoryData data;
    {
      FileReader reader(binFilePath);
      kml::binary::DeserializerKml des(data, {});
      des.Deserialize(reader);
    }
    auto ts3 = std::chrono::steady_clock::now();

    auto d1 = std::chrono::duration_cast<std::chrono::nanoseconds>(ts2 - ts1).count();
    auto d2 = std::chrono::duration_cast<std::chrono::nanoseconds>(ts3 - ts2).count();
    avg += (static_cast<double>(d1) / static_cast<double>(d2));

    SpeedTestStat st;
    st.bookmarksCount = data.m_bookmarksData.size() + data.m_tracksData.size();
    st.d1 = d1;
    st.d2 = d2;
    stats.push_back(st);
  }
  avg /= files.size();

  LOG(LINFO, ("Avg speed coef =", avg));

  auto const v1 = CalculateAvgCoef(stats, [](SpeedTestStat const & s) { return s.bookmarksCount < 100; });
  auto const v2 = CalculateAvgCoef(stats, [](SpeedTestStat const & s) { return s.bookmarksCount >= 100 && s.bookmarksCount < 1000; });
  auto const v3 = CalculateAvgCoef(stats, [](SpeedTestStat const & s) { return s.bookmarksCount >= 1000; });

  LOG(LINFO, ("Small: ", v1.second, "Avg speed coef =", v1.first));
  LOG(LINFO, ("Medium: ", v2.second, "Avg speed coef =", v2.first));
  LOG(LINFO, ("Large: ", v3.second, "Avg speed coef =", v3.first));
}