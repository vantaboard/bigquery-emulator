#include "backend/engine/control/gcs_uri_resolver.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace fs = std::filesystem;

namespace {

std::string StorageEmulatorOrigin() {
  const char* host = std::getenv("STORAGE_EMULATOR_HOST");
  if (host == nullptr || host[0] == '\0') {
    const char* port = std::getenv("FAKE_GCS_PORT");
    if (port == nullptr || port[0] == '\0') {
      port = "4443";
    }
    return absl::StrCat("http://127.0.0.1:", port);
  }
  std::string out(host);
  if (absl::StartsWith(out, "http://") || absl::StartsWith(out, "https://")) {
    return out;
  }
  if (absl::StartsWith(out, "//")) {
    return absl::StrCat("http:", out);
  }
  const char* port = std::getenv("FAKE_GCS_PORT");
  if (port == nullptr || port[0] == '\0') {
    port = "4443";
  }
  if (out.find(':') != std::string::npos) {
    return absl::StrCat("http://", out);
  }
  return absl::StrCat("http://", out, ":", port);
}

std::string UrlEscapePath(absl::string_view s) {
  std::string out;
  out.reserve(s.size() * 3);
  for (unsigned char c : s) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
        ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' ||
        c == '~') {
      out.push_back(static_cast<char>(c));
      continue;
    }
    if (c == '/') {
      out.append("%2F");
      continue;
    }
    char hex[4];
    std::snprintf(hex, sizeof(hex), "%%%02X", c);
    out.append(hex);
  }
  return out;
}

std::string GcsMediaURL(absl::string_view origin, const GCSObjectParts& parts) {
  return absl::StrCat(origin,
                      "/storage/v1/b/",
                      UrlEscapePath(parts.bucket),
                      "/o/",
                      UrlEscapePath(parts.object),
                      "?alt=media");
}

std::string GcsUploadURL(absl::string_view origin,
                         const GCSObjectParts& parts) {
  return absl::StrCat(origin,
                      "/upload/storage/v1/b/",
                      UrlEscapePath(parts.bucket),
                      "/o?uploadType=media&name=",
                      UrlEscapePath(parts.object));
}

std::string ShellEscape(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "'\\''"}});
}

absl::Status RunShellCommand(const std::string& cmd) {
  const int rc = std::system(cmd.c_str());
  if (rc != 0) {
    return absl::InternalError(
        absl::StrCat("shell command failed (rc=", rc, "): ", cmd));
  }
  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<GCSObjectParts> ParseGsUri(absl::string_view uri) {
  if (!absl::StartsWith(uri, "gs://")) {
    return absl::InvalidArgumentError(
        absl::StrCat("expected gs:// uri, got '", uri, "'"));
  }
  const absl::string_view rest = uri.substr(strlen("gs://"));
  const size_t slash = rest.find('/');
  if (slash == absl::string_view::npos || slash == 0 ||
      slash == rest.size() - 1) {
    return absl::InvalidArgumentError(
        absl::StrCat("invalid gs:// uri: '", uri, "'"));
  }
  GCSObjectParts parts;
  parts.bucket = std::string(rest.substr(0, slash));
  parts.object = std::string(rest.substr(slash + 1));
  return parts;
}

std::string GcsCachePath(absl::string_view data_dir,
                         const GCSObjectParts& parts) {
  return (fs::path(std::string(data_dir)) / "external" / "gcs-cache" /
          parts.bucket / parts.object)
      .string();
}

absl::StatusOr<std::string> MaterializeGCSObjectToCache(
    absl::string_view gs_uri, absl::string_view data_dir) {
  if (data_dir.empty()) {
    return absl::InvalidArgumentError(
        "control op executor: data_dir is required to resolve gs:// URIs");
  }
  absl::StatusOr<GCSObjectParts> parts = ParseGsUri(gs_uri);
  if (!parts.ok()) return parts.status();
  const std::string cache = GcsCachePath(data_dir, *parts);
  if (fs::exists(cache)) {
    return cache;
  }
  const std::string origin = StorageEmulatorOrigin();
  const std::string url = GcsMediaURL(origin, *parts);
  std::error_code ec;
  fs::create_directories(fs::path(cache).parent_path(), ec);
  const std::string cmd = absl::StrCat(
      "curl -sf '", ShellEscape(url), "' -o '", ShellEscape(cache), "'");
  if (absl::Status fetched = RunShellCommand(cmd); !fetched.ok()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "control op executor: could not fetch ",
        gs_uri,
        " from storage emulator (set STORAGE_EMULATOR_HOST or pre-stage ",
        cache,
        "): ",
        fetched.message()));
  }
  if (!fs::exists(cache)) {
    return absl::InternalError(
        absl::StrCat("control op executor: fetch produced no file at ", cache));
  }
  return cache;
}

absl::Status UploadGCSObjectFromFile(absl::string_view gs_uri,
                                     absl::string_view local_path,
                                     absl::string_view content_type) {
  absl::StatusOr<GCSObjectParts> parts = ParseGsUri(gs_uri);
  if (!parts.ok()) return parts.status();
  if (!fs::exists(std::string(local_path))) {
    return absl::NotFoundError(
        absl::StrCat("export local file missing: ", local_path));
  }
  const std::string origin = StorageEmulatorOrigin();
  const std::string url = GcsUploadURL(origin, *parts);
  std::string ct = std::string(content_type);
  if (ct.empty()) {
    ct = "application/octet-stream";
  }
  const std::string cmd = absl::StrCat("curl -sf -X POST -H 'Content-Type: ",
                                       ShellEscape(ct),
                                       "' --data-binary ",
                                       "@'",
                                       ShellEscape(local_path),
                                       "' '",
                                       ShellEscape(url),
                                       "'");
  return RunShellCommand(cmd);
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
