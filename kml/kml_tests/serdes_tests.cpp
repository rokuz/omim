#include "testing/testing.hpp"

#include "kml/serdes.hpp"
#include "kml/serdes_binary.hpp"

#include "coding/file_reader.hpp"
#include "coding/file_writer.hpp"
#include "coding/reader.hpp"
#include "coding/writer.hpp"

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

UNIT_TEST(Kml_File_Deserialization)
{
  kml::CategoryData data;
  try
  {
    KmlFileDeserializer des("/Users/romankuznetsov/Dev/Projects/omim/data/big_test.kml");
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
    FileWriter file("/Users/romankuznetsov/Dev/Projects/omim/data/big_test.kmb");
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
    KmlFileDeserializer des("/Users/romankuznetsov/Dev/Projects/omim/data/big_test.kml");
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
  FileReader reader("/Users/romankuznetsov/Dev/Projects/omim/data/big_test.kmb");
  kml::binary::DeserializerKml des(data, {});
  des.Deserialize(reader);
}