#include "backend/engine/semantic/functions/hll_plus_plus.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/strings/escaping.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace hll_plus_plus {
namespace {

struct InitCase {
  int precision;
  uint64_t seed;
  int count;
};

struct Merge {
  InitCase first;
  InitCase second;
};

struct Golden {
  const char* name;
  const char* base64;
  size_t size;
  uint64_t digest;
  int64_t estimate;
};

std::vector<uint64_t> SplitMix64(uint64_t seed, int count) {
  std::vector<uint64_t> out;
  uint64_t state = seed;
  for (int i = 0; i < count; ++i) {
    state += 0x9E3779B97F4A7C15ULL;
    uint64_t z = state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    out.push_back(z ^ (z >> 31));
  }
  return out;
}

uint64_t Fnv1a(const std::string& bytes) {
  uint64_t hash = 0xcbf29ce484222325ULL;
  for (char c : bytes) {
    hash = (hash ^ static_cast<uint8_t>(c)) * 0x100000001b3ULL;
  }
  return hash;
}

Sketch Init(const InitCase& c) {
  auto sketch =
      Sketch::Create(c.precision,
                     std::min(c.precision + 5, kMaximumSparsePrecision),
                     kValueTypeUint64);
  EXPECT_TRUE(sketch.ok()) << sketch.status();
  for (uint64_t hash : SplitMix64(c.seed, c.count))
    sketch->AddHash(hash);
  return *std::move(sketch);
}

Sketch Parsed(Sketch sketch) {
  auto parsed = Sketch::Parse(sketch.Serialize());
  EXPECT_TRUE(parsed.ok()) << parsed.status();
  return *std::move(parsed);
}

void ExpectGolden(Sketch sketch, const Golden& golden) {
  const std::string bytes = sketch.Serialize();
  EXPECT_EQ(bytes.size(), golden.size) << golden.name;
  EXPECT_EQ(Fnv1a(bytes), golden.digest) << golden.name;
  if (golden.base64[0] != '\0') {
    EXPECT_EQ(absl::Base64Escape(bytes), golden.base64) << golden.name;
  }
  EXPECT_EQ(Parsed(sketch).Estimate(), golden.estimate) << golden.name;
}

const InitCase kInitCases[] = {
    {15, 1, 0},
    {15, 1, 1},
    {15, 2, 10},
    {15, 3, 300},
    {15, 4, 3000},
    {15, 5, 5500},
    {15, 6, 6200},
    {15, 7, 30000},
    {15, 8, 200000},
    {10, 9, 50},
    {10, 10, 2000},
    {24, 11, 1000},
    {18, 12, 100000},
};

const Merge kMerges[] = {
    {{15, 3, 300}, {15, 13, 200}},
    {{15, 3, 300}, {15, 7, 30000}},
    {{15, 7, 30000}, {15, 3, 300}},
    {{15, 7, 30000}, {15, 14, 30000}},
    {{15, 4, 3000}, {15, 15, 3000}},
    {{15, 3, 300}, {10, 9, 50}},
    {{15, 7, 30000}, {10, 10, 2000}},
    {{10, 10, 2000}, {15, 7, 30000}},
    {{10, 9, 50}, {15, 3, 300}},
    {{24, 11, 1000}, {15, 4, 3000}},
};

const Golden kInitGoldens[] = {
    {"InitCase{15, 1, 0}",
     "CHAQABgCIAiCBwYYDyAUMgA=",
     17,
     0x2d51583225a0fdf5ULL,
     0},
    {"InitCase{15, 1, 1}",
     "CHAQARgCIAiCBwsQARgPIBQyA6KhJA==",
     22,
     0xc49f249e8b721414ULL,
     1},
    {"InitCase{15, 2, 10}",
     "CHAQChgCIAiCByIQChgPIBQyGseCEP31A/+eAsDTD7gk6a4IrAr9XbdSpIUB",
     45,
     0x49e87796d1863c36ULL,
     10},
    {"InitCase{15, 3, 300}", "", 630, 0xde573df11b4f5f9ULL, 300},
    {"InitCase{15, 4, 3000}", "", 5211, 0x52a982670d5b1b8cULL, 3001},
    {"InitCase{15, 5, 5500}", "", 8496, 0x6e5c645a34827e6aULL, 5504},
    {"InitCase{15, 6, 6200}", "", 9327, 0x92e2b113c12ee009ULL, 6203},
    {"InitCase{15, 7, 30000}", "", 32791, 0xfc4c04020fd7f6eULL, 30196},
    {"InitCase{15, 8, 200000}", "", 32791, 0xacb48f20ff38109ULL, 198852},
    {"InitCase{10, 9, 50}",
     "CHAQMhgCIAiCB2YQMhgKIA8yXh7mDOQBkATjAcsG0AE1jRXFBIwB0APpAQLyBGVb3wqGEYcL4"
     "wGxCYgCZs4NqQqhBrgEpAfRAs8FmwLEAUDSBK0MsgKsBo8BhwqwBp0EiwTJCM0K7AVJ1KECwe"
     "oCggU=",
     113,
     0x652bb6b0c15218d0ULL,
     50},
    {"InitCase{10, 10, 2000}", "", 1044, 0x6631e9db2fd70110ULL, 2018},
    {"InitCase{24, 11, 1000}", "", 3094, 0xf31902850646ec3ULL, 1000},
    {"InitCase{18, 12, 100000}", "", 124866, 0xaec05b43d11bb50eULL, 99973},
};

const Golden kMergeGoldens[] = {
    {"Merge{InitCase{15, 3, 300}, InitCase{15, 13, 200}}",
     "",
     1006,
     0x75112de06e2c81e9ULL,
     498},
    {"Merge{InitCase{15, 3, 300}, InitCase{15, 7, 30000}}",
     "",
     32791,
     0x85410efbecab3680ULL,
     30439},
    {"Merge{InitCase{15, 7, 30000}, InitCase{15, 3, 300}}",
     "",
     32791,
     0x85410efbecab3680ULL,
     30439},
    {"Merge{InitCase{15, 7, 30000}, InitCase{15, 14, 30000}}",
     "",
     32791,
     0x4fa1940cda56265eULL,
     59738},
    {"Merge{InitCase{15, 4, 3000}, InitCase{15, 15, 3000}}",
     "",
     9107,
     0x1382f2c76b80edeaULL,
     6002},
    {"Merge{InitCase{15, 3, 300}, InitCase{10, 9, 50}}",
     "CHAQ3gIYAiAIggfHAxDdAhgKIA8yvQMLE0Z72QIRjgGTAVIXUiUnkwTkATYcC00yGDTDAQ4QB"
     "xkeBUBnsAEk1gJpWwNa0AEPJs8BU/8CNzxvTFh1FRhnRJoCRgp/"
     "HTc4K58BpwEsRVIDDg4PFiwLBNsDWyEWHiYRJGzAAukBAhQDAxlGBvMDYwJbugGFAaYB3AHBA"
     "jshB28aSCkHLZQBPBMbGJkBFhAxe3o/"
     "PycwNyZ0pAEenAGkAcIBYwgFHFIPOlmrAUx2FhERnAE1uwKJAgcvBD1QVnQac3LNAaABLLQCc"
     "QGHAmY13QHiARppqwQh6wEIEn8wRhEEkAIczAEjbm2GAxdpKf8BfVkNhgI5iAEvRQmjAZABHA"
     "wua6cBShQxqQMEEgWcAQg3G0QqZ/"
     "ABDFDyAQsKBSMBogE7Z10XKeQCNw8lPUZjKi4XZHc0CQy3ASmzAjjXAoQBERtSpQEgygGSAgr"
     "GAoUBCq8CEFmfAbIB9gGEAiSVAa8DDGUIcyQEFST8AhAtAzoKxgGpAVglNAYIJ5wCvAMlDDlY"
     "IRoOM3IunwIPcwO/"
     "ASKEASmgAmk1OwhG7gEPKQ+QASUk1KECwHaAKMAhwAeBowHDAr8CgAK9JYIV",
     468,
     0x19ca5ad17a908f0ULL,
     351},
    {"Merge{InitCase{15, 7, 30000}, InitCase{10, 10, 2000}}",
     "",
     1045,
     0x97c00bfccc419037ULL,
     34363},
    {"Merge{InitCase{10, 10, 2000}, InitCase{15, 7, 30000}}",
     "",
     1045,
     0x97c00bfccc419037ULL,
     34363},
    {"Merge{InitCase{10, 9, 50}, InitCase{15, 3, 300}}",
     "CHAQ3gIYAiAIggfHAxDdAhgKIA8yvQMLE0Z72QIRjgGTAVIXUiUnkwTkATYcC00yGDTDAQ4QB"
     "xkeBUBnsAEk1gJpWwNa0AEPJs8BU/8CNzxvTFh1FRhnRJoCRgp/"
     "HTc4K58BpwEsRVIDDg4PFiwLBNsDWyEWHiYRJGzAAukBAhQDAxlGBvMDYwJbugGFAaYB3AHBA"
     "jshB28aSCkHLZQBPBMbGJkBFhAxe3o/"
     "PycwNyZ0pAEenAGkAcIBYwgFHFIPOlmrAUx2FhERnAE1uwKJAgcvBD1QVnQac3LNAaABLLQCc"
     "QGHAmY13QHiARppqwQh6wEIEn8wRhEEkAIczAEjbm2GAxdpKf8BfVkNhgI5iAEvRQmjAZABHA"
     "wua6cBShQxqQMEEgWcAQg3G0QqZ/"
     "ABDFDyAQsKBSMBogE7Z10XKeQCNw8lPUZjKi4XZHc0CQy3ASmzAjjXAoQBERtSpQEgygGSAgr"
     "GAoUBCq8CEFmfAbIB9gGEAiSVAa8DDGUIcyQEFST8AhAtAzoKxgGpAVglNAYIJ5wCvAMlDDlY"
     "IRoOM3IunwIPcwO/"
     "ASKEASmgAmk1OwhG7gEPKQ+QASUk1KECwHaAKMAhwAeBowHDAr8CgAK9JYIV",
     468,
     0x19ca5ad17a908f0ULL,
     351},
    {"Merge{InitCase{24, 11, 1000}, InitCase{15, 4, 3000}}",
     "",
     6630,
     0x58530c1c334d2d2bULL,
     4001},
};

TEST(HllPlusPlusTest, InitMatchesZetasketch) {
  for (size_t i = 0; i < std::size(kInitCases); ++i) {
    ExpectGolden(Init(kInitCases[i]), kInitGoldens[i]);
  }
}

TEST(HllPlusPlusTest, MergeMatchesZetasketch) {
  for (size_t i = 0; i < std::size(kMerges); ++i) {
    Sketch merged = Parsed(Init(kMerges[i].first));
    ASSERT_TRUE(merged.Merge(Parsed(Init(kMerges[i].second))).ok());
    ExpectGolden(std::move(merged), kMergeGoldens[i]);
  }
}

TEST(HllPlusPlusTest, ParseKeepsTheBytes) {
  for (const InitCase& c : kInitCases) {
    std::string bytes = Init(c).Serialize();
    EXPECT_EQ(Parsed(Init(c)).Serialize(), bytes);
  }
}

TEST(HllPlusPlusTest, MergeRejectsDifferentValueTypes) {
  auto longs = Sketch::Create(15, 20, kValueTypeUint64);
  auto strings = Sketch::Create(15, 20, kValueTypeBytesOrUtf8String);
  ASSERT_TRUE(longs.ok() && strings.ok());
  EXPECT_FALSE(longs->Merge(*strings).ok());
}

TEST(HllPlusPlusTest, ParseRejectsOtherBytes) {
  EXPECT_FALSE(Sketch::Parse("\x12\xef\x7f").ok());
  EXPECT_FALSE(Sketch::Parse("not a sketch").ok());
}

}  // namespace
}  // namespace hll_plus_plus
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
