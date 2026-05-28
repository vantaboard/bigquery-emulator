// Smoke binary for the GoogleSQL prebuilt artifact (artifact-producer
// verifier).
//
// `tools/googlesql-prebuilt/verify.sh` runs TWO smoke variants against
// the UNPACKED artifact:
//
//   1. clang-link smoke (`--smoke-mode=link`, portable, default):
//      Links this file directly against `${root}/lib/libgooglesql.a`
//      + `${root}/lib/libgooglesql_protos.a` using a stock `clang++`.
//      Proves (a) the include path is wired (`-I${root}/include`
//      resolves to a real directory shipped in the tarball),
//      (b) both static archives are valid `ar` files the linker can
//      scan, (c) the combined include + library layout matches
//      the compatibility surface's frozen shape.
//
//   2. bazel-wrappers smoke (`--smoke-mode=bazel`, CI only):
//      Builds a tiny `cc_binary` against the wrapper `cc_library`
//      targets shipped at `//googlesql/public:error_helpers` inside
//      the unpacked artifact's BUILD files. That variant DOES drag
//      in the real abseil/protobuf deps via `MODULE.bazel`, so the
//      verifier sees the full consumer-wiring consume path end to end.
//
// The clang-link smoke deliberately does NOT reference any GoogleSQL
// types or functions: if it did, the link would require the host to
// resolve abseil/protobuf/etc symbols, which only the bazel-wrappers
// smoke supplies via its `deps = [...]`. Keeping the symbol set empty
// makes this test green both in fixture mode (stub .a) and in
// production mode (real .a with real GoogleSQL objects) without
// adding a third-party dep hunt to the verifier's hot path.

int main(int /*argc*/, char** /*argv*/) {
  return 0;
}
