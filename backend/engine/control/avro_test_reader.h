#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_TEST_READER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_TEST_READER_H_

// Minimal Avro Object Container File reader for tests, written against
// the Avro 1.11 spec on purpose so it shares no code with the writer
// under test (`avro_writer.cc`).

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "gtest/gtest.h"
#include "zlib.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace avro_test {

class AvroCursor {
 public:
  explicit AvroCursor(absl::string_view data) : data_(data) {}

  bool AtEnd() const {
    return pos_ >= data_.size();
  }

  int64_t ReadLong() {
    uint64_t raw = 0;
    int shift = 0;
    while (true) {
      const auto b = static_cast<uint8_t>(data_.at(pos_++));
      raw |= static_cast<uint64_t>(b & 0x7f) << shift;
      if ((b & 0x80) == 0) break;
      shift += 7;
    }
    return static_cast<int64_t>(raw >> 1) ^ -static_cast<int64_t>(raw & 1);
  }

  std::string ReadFixed(size_t n) {
    std::string out(data_.substr(pos_, n));
    pos_ += n;
    return out;
  }

  std::string ReadBytes() {
    return ReadFixed(static_cast<size_t>(ReadLong()));
  }

  bool ReadBool() {
    return data_.at(pos_++) != 0;
  }

  double ReadDouble() {
    double v = 0;
    std::memcpy(&v, data_.data() + pos_, sizeof(v));
    pos_ += sizeof(v);
    return v;
  }

  std::vector<std::string> ReadStringArray() {
    std::vector<std::string> out;
    while (true) {
      int64_t count = ReadLong();
      if (count == 0) break;
      if (count < 0) {
        count = -count;
        ReadLong();
      }
      for (int64_t i = 0; i < count; ++i)
        out.push_back(ReadBytes());
    }
    return out;
  }

 private:
  absl::string_view data_;
  size_t pos_ = 0;
};

inline std::string SnappyDecode(absl::string_view in) {
  size_t pos = 0;
  uint64_t expected = 0;
  for (int shift = 0;; shift += 7) {
    const auto b = static_cast<uint8_t>(in[pos++]);
    expected |= static_cast<uint64_t>(b & 0x7f) << shift;
    if ((b & 0x80) == 0) break;
  }
  std::string out;
  auto read_le = [&](int n) {
    uint32_t v = 0;
    for (int i = 0; i < n; ++i) {
      v |= static_cast<uint32_t>(static_cast<uint8_t>(in[pos++])) << (8 * i);
    }
    return v;
  };
  while (pos < in.size()) {
    const auto tag = static_cast<uint8_t>(in[pos++]);
    uint32_t len = 0;
    uint32_t offset = 0;
    switch (tag & 3) {
      case 0:
        len = tag >> 2;
        if (len >= 60) len = read_le(static_cast<int>(len - 59));
        out.append(in.substr(pos, len + 1));
        pos += len + 1;
        continue;
      case 1:
        len = 4 + ((tag >> 2) & 7);
        offset = ((tag >> 5) << 8) | read_le(1);
        break;
      case 2:
        len = (tag >> 2) + 1;
        offset = read_le(2);
        break;
      default:
        len = (tag >> 2) + 1;
        offset = read_le(4);
        break;
    }
    EXPECT_GT(offset, 0u);
    EXPECT_LE(offset, out.size());
    for (uint32_t i = 0; i < len; ++i)
      out.push_back(out[out.size() - offset]);
  }
  EXPECT_EQ(out.size(), expected);
  return out;
}

inline std::string RawInflate(absl::string_view in) {
  z_stream zs{};
  EXPECT_EQ(inflateInit2(&zs, -15), Z_OK);
  zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(in.data()));
  zs.avail_in = static_cast<uInt>(in.size());
  std::string out;
  char buf[4096];
  int rc = Z_OK;
  while (rc != Z_STREAM_END) {
    zs.next_out = reinterpret_cast<Bytef*>(buf);
    zs.avail_out = sizeof(buf);
    rc = inflate(&zs, Z_NO_FLUSH);
    EXPECT_TRUE(rc == Z_OK || rc == Z_STREAM_END) << rc;
    if (rc != Z_OK && rc != Z_STREAM_END) break;
    out.append(buf, sizeof(buf) - zs.avail_out);
  }
  inflateEnd(&zs);
  return out;
}

struct AvroFile {
  std::string schema;
  std::string codec;
  int64_t row_count = 0;
  std::string data;
};

inline AvroFile ReadAvroFile(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  EXPECT_TRUE(in.good()) << "missing export file " << path;
  const std::string raw((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
  AvroCursor cur(raw);
  AvroFile file;
  EXPECT_EQ(cur.ReadFixed(4), std::string("Obj\x01", 4));
  while (true) {
    int64_t count = cur.ReadLong();
    if (count == 0) break;
    if (count < 0) {
      count = -count;
      cur.ReadLong();
    }
    for (int64_t i = 0; i < count; ++i) {
      const std::string key = cur.ReadBytes();
      const std::string value = cur.ReadBytes();
      if (key == "avro.schema") file.schema = value;
      if (key == "avro.codec") file.codec = value;
    }
  }
  const std::string sync = cur.ReadFixed(16);
  while (!cur.AtEnd()) {
    file.row_count += cur.ReadLong();
    const std::string block = cur.ReadBytes();
    if (file.codec == "deflate") {
      file.data += RawInflate(block);
    } else if (file.codec == "snappy") {
      const std::string body = block.substr(0, block.size() - 4);
      std::string decoded = SnappyDecode(body);
      const uLong crc = crc32(0L,
                              reinterpret_cast<const Bytef*>(decoded.data()),
                              static_cast<uInt>(decoded.size()));
      const auto* tail =
          reinterpret_cast<const uint8_t*>(block.data() + block.size() - 4);
      const uint32_t stored = (uint32_t{tail[0]} << 24) |
                              (uint32_t{tail[1]} << 16) |
                              (uint32_t{tail[2]} << 8) | uint32_t{tail[3]};
      EXPECT_EQ(stored, static_cast<uint32_t>(crc));
      file.data += decoded;
    } else {
      file.data += block;
    }
    EXPECT_EQ(cur.ReadFixed(16), sync);
  }
  return file;
}

}  // namespace avro_test
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_AVRO_TEST_READER_H_
