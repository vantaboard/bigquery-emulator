# Duktape (embedded JavaScript engine)

Duktape powers BigQuery `LANGUAGE js` scalar UDF call-time evaluation in the
semantic executor (`backend/engine/semantic/js_udf_runtime.cc`).

The upstream tarball is fetched by `MODULE.bazel` (`http_archive` name
`duktape_src`). This directory exposes a stable `//third_party/duktape:duktape`
label for engine targets.

Pin and checksum live in `VERSION`.
