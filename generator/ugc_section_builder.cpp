#include "generator/ugc_section_builder.hpp"

#include "generator/ugc_translator.hpp"
#include "generator/utils.hpp"

#include "ugc/binary/index_ugc.hpp"
#include "ugc/binary/serdes.hpp"

#include "indexer/feature_data.hpp"
#include "indexer/feature_processor.hpp"
#include "indexer/ftraits.hpp"

#include "base/geo_object_id.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

namespace generator
{
bool BuildUgcMwmSection(std::string const & srcDbFilename, std::string const & mwmFile,
                        std::string const & osmToFeatureFilename)
{
  using ugc::binary::IndexUGC;

  LOG(LINFO, ("Build UGC section"));

  std::unordered_map<uint32_t, std::vector<base::GeoObjectId>> featureToOsmId;
  if (!ParseFeatureIdToOsmIdMapping(osmToFeatureFilename, featureToOsmId))
    return false;

  UGCTranslator translator(srcDbFilename);

  std::vector<IndexUGC> content;

  feature::ForEachFromDat(mwmFile, [&](FeatureType & f, uint32_t featureId) {
    auto const optItem = ftraits::UGC::GetValue(feature::TypesHolder(f));
    if (!optItem)
      return;

    if (!ftraits::UGC::IsUGCAvailable(optItem->m_mask))
      return;

    auto const it = featureToOsmId.find(featureId);
    CHECK(it != featureToOsmId.cend() && it->second.size() != 0,
          ("FeatureID", featureId, "is not found in", osmToFeatureFilename));

    ugc::UGC result;
    if (!translator.TranslateUGC(it->second[0], result))
      return;

    if (result.IsEmpty())
      return;

    content.emplace_back(featureId, result);
  });

  if (content.empty())
    return true;

  FilesContainerW cont(mwmFile, FileWriter::OP_WRITE_EXISTING);
  FileWriter writer = cont.GetWriter(UGC_FILE_TAG);
  ugc::binary::UGCSeriaizer serializer(std::move(content));
  serializer.Serialize(writer);

  return true;
}
}  // namespace generator
