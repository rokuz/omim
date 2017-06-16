#pragma once

#include "std/array.hpp"
#include "std/string.hpp"

// This file is autogenerated while exporting sounds.csv from the google table.
// It contains the list of languages which can be used by TTS.
// It shall be included to Android(jni) and iOS part to get the languages list.

namespace routing
{
namespace turns
{
namespace sound
{
array<string, 28> const kLanguageList =
{{
  "ar",
  "cs",
  "da",
  "de",
  "el",
  "en",
  "es",
  "fi",
  "fr",
  "hi",
  "hr",
  "hu",
  "id",
  "it",
  "ja",
  "ko",
  "nl",
  "pl",
  "pt",
  "ro",
  "ru",
  "sk",
  "sv",
  "sw",
  "th",
  "tr",
  "zh-Hans",
  "zh-Hant"
}};
}  // namespace sound
}  // namespace turns
}  // namespace routing
