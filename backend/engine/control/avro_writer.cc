#include "backend/engine/control/avro_writer.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/control/avro_encoding.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "zlib.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace {

constexpr size_t kBlockBytes = 64 * 1024;

void AppendSnappyLiteral(std::string* out, absl::string_view literal) {
  if (literal.empty()) return;
  const auto n = static_cast<uint32_t>(literal.size() - 1);
  if (n < 60) {
    out->push_back(static_cast<char>(n << 2));
  } else {
    const int width = n < (1U << 8)    ? 1
                      : n < (1U << 16) ? 2
                      : n < (1U << 24) ? 3
                                       : 4;
    out->push_back(static_cast<char>((59 + width) << 2));
    for (int i = 0; i < width; ++i) {
      out->push_back(static_cast<char>((n >> (8 * i)) & 0xff));
    }
  }
  out->append(literal.data(), literal.size());
}

std::string SnappyCompress(absl::string_view in) {
  std::string out;
  for (auto n = static_cast<uint32_t>(in.size());; n >>= 7) {
    if (n < 0x80) {
      out.push_back(static_cast<char>(n));
      break;
    }
    out.push_back(static_cast<char>((n & 0x7f) | 0x80));
  }
  constexpr int kHashBits = 14;
  std::vector<int64_t> table(size_t{1} << kHashBits, -1);
  auto load32 = [&in](size_t pos) {
    uint32_t v = 0;
    std::memcpy(&v, in.data() + pos, sizeof(v));
    return v;
  };
  size_t literal_start = 0;
  size_t i = 0;
  while (i + 4 <= in.size()) {
    const uint32_t current = load32(i);
    const uint32_t slot = (current * 0x1e35a7bdU) >> (32 - kHashBits);
    const int64_t previous = table[slot];
    table[slot] = static_cast<int64_t>(i);
    const auto candidate = static_cast<size_t>(previous);
    if (previous < 0 || i - candidate > 0xffff ||
        load32(candidate) != current) {
      ++i;
      continue;
    }
    AppendSnappyLiteral(&out, in.substr(literal_start, i - literal_start));
    size_t len = 4;
    while (len < 64 && i + len < in.size() &&
           in[candidate + len] == in[i + len]) {
      ++len;
    }
    const size_t offset = i - candidate;
    out.push_back(static_cast<char>(((len - 1) << 2) | 2));
    out.push_back(static_cast<char>(offset & 0xff));
    out.push_back(static_cast<char>(offset >> 8));
    i += len;
    literal_start = i;
  }
  AppendSnappyLiteral(&out, in.substr(literal_start));
  return out;
}

absl::StatusOr<std::string> RawDeflate(absl::string_view in) {
  z_stream zs{};
  if (deflateInit2(
          &zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) !=
      Z_OK) {
    return absl::InternalError("EXPORT DATA: deflateInit2 failed");
  }
  std::string out(deflateBound(&zs, static_cast<uLong>(in.size())), '\0');
  zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(in.data()));
  zs.avail_in = static_cast<uInt>(in.size());
  zs.next_out = reinterpret_cast<Bytef*>(out.data());
  zs.avail_out = static_cast<uInt>(out.size());
  const int rc = deflate(&zs, Z_FINISH);
  deflateEnd(&zs);
  if (rc != Z_STREAM_END) {
    return absl::InternalError(
        absl::StrCat("EXPORT DATA: deflate failed (rc=", rc, ")"));
  }
  out.resize(zs.total_out);
  return out;
}

absl::string_view CodecName(AvroCodec codec) {
  switch (codec) {
    case AvroCodec::kDeflate:
      return "deflate";
    case AvroCodec::kSnappy:
      return "snappy";
    case AvroCodec::kNull:
      break;
  }
  return "null";
}

}  // namespace

absl::StatusOr<AvroCodec> ParseAvroCompression(absl::string_view compression) {
  const std::string upper = absl::AsciiStrToUpper(compression);
  if (upper.empty() || upper == "NONE") return AvroCodec::kNull;
  if (upper == "DEFLATE") return AvroCodec::kDeflate;
  if (upper == "SNAPPY") return AvroCodec::kSnappy;
  return absl::InvalidArgumentError(
      absl::StrCat("EXPORT DATA: compression '",
                   compression,
                   "' is not supported for AVRO; use DEFLATE or SNAPPY"));
}

AvroFileWriter::AvroFileWriter(schema::TableSchema columns, Options options)
    : columns_(std::move(columns)), options_(options) {}

absl::StatusOr<std::unique_ptr<AvroFileWriter>> AvroFileWriter::Create(
    const std::string& path,
    const schema::TableSchema& columns,
    Options options) {
  absl::StatusOr<std::string> schema_json =
      BuildAvroSchemaJson(columns, options.use_logical_types);
  if (!schema_json.ok()) return schema_json.status();
  std::unique_ptr<AvroFileWriter> writer(new AvroFileWriter(columns, options));
  writer->out_.open(path, std::ios::binary | std::ios::trunc);
  if (!writer->out_.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("EXPORT DATA: cannot open '", path, "' for writing"));
  }
  std::random_device rd;
  std::mt19937_64 rng((uint64_t{rd()} << 32) | rd());
  for (int i = 0; i < 2; ++i) {
    const uint64_t bits = rng();
    writer->sync_.append(reinterpret_cast<const char*>(&bits), sizeof(bits));
  }
  std::string header("Obj\x01", 4);
  AppendAvroLong(&header, 2);
  AppendAvroBytes(&header, "avro.schema");
  AppendAvroBytes(&header, *schema_json);
  AppendAvroBytes(&header, "avro.codec");
  AppendAvroBytes(&header, CodecName(options.codec));
  AppendAvroLong(&header, 0);
  header += writer->sync_;
  writer->out_.write(header.data(),
                     static_cast<std::streamsize>(header.size()));
  if (!writer->out_.good()) {
    return absl::InternalError(
        absl::StrCat("EXPORT DATA: failed to write Avro header to ", path));
  }
  return writer;
}

absl::Status AvroFileWriter::Write(const storage::Row& row) {
  if (row.cells.size() != columns_.columns.size()) {
    return absl::InternalError(absl::StrCat("EXPORT DATA: row has ",
                                            row.cells.size(),
                                            " cells but the schema has ",
                                            columns_.columns.size(),
                                            " columns"));
  }
  for (size_t i = 0; i < row.cells.size(); ++i) {
    absl::Status s = AppendAvroField(
        &block_, columns_.columns[i], row.cells[i], options_.use_logical_types);
    if (!s.ok()) return s;
  }
  ++block_rows_;
  if (block_.size() >= kBlockBytes) return FlushBlock();
  return absl::OkStatus();
}

absl::Status AvroFileWriter::FlushBlock() {
  if (block_rows_ == 0) return absl::OkStatus();
  std::string payload;
  switch (options_.codec) {
    case AvroCodec::kNull:
      payload = std::move(block_);
      break;
    case AvroCodec::kDeflate: {
      absl::StatusOr<std::string> deflated = RawDeflate(block_);
      if (!deflated.ok()) return deflated.status();
      payload = *std::move(deflated);
      break;
    }
    case AvroCodec::kSnappy: {
      payload = SnappyCompress(block_);
      const auto crc = static_cast<uint32_t>(
          crc32(0L,
                reinterpret_cast<const Bytef*>(block_.data()),
                static_cast<uInt>(block_.size())));
      for (int shift = 24; shift >= 0; shift -= 8) {
        payload.push_back(static_cast<char>((crc >> shift) & 0xff));
      }
      break;
    }
  }
  std::string framing;
  AppendAvroLong(&framing, block_rows_);
  AppendAvroLong(&framing, static_cast<int64_t>(payload.size()));
  out_.write(framing.data(), static_cast<std::streamsize>(framing.size()));
  out_.write(payload.data(), static_cast<std::streamsize>(payload.size()));
  out_.write(sync_.data(), static_cast<std::streamsize>(sync_.size()));
  block_.clear();
  block_rows_ = 0;
  if (!out_.good()) {
    return absl::InternalError("EXPORT DATA: failed to write Avro block");
  }
  return absl::OkStatus();
}

absl::Status AvroFileWriter::Finish() {
  absl::Status flushed = FlushBlock();
  if (!flushed.ok()) return flushed;
  out_.close();
  if (out_.fail()) {
    return absl::InternalError("EXPORT DATA: failed to close Avro file");
  }
  return absl::OkStatus();
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
