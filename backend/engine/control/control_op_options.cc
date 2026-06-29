

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace {

std::optional<std::string> TryLiteralString(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != ::googlesql::RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr) return std::nullopt;
  const ::googlesql::Value& v = lit->value();
  if (v.is_null() || v.type_kind() != ::googlesql::TYPE_STRING) {
    return std::nullopt;
  }
  return v.string_value();
}

}  // namespace

absl::StatusOr<std::string> OptionStringValue(
    const ::googlesql::ResolvedOption* opt) {
  if (opt == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: option entry is null");
  }
  if (auto s = TryLiteralString(opt->value()); s.has_value()) {
    return *s;
  }
  return absl::InvalidArgumentError(
      absl::StrCat("control op executor: option '",
                   opt->name(),
                   "' must be a STRING literal"));
}

absl::StatusOr<std::string> FindOptionString(
    const std::vector<std::unique_ptr<const ::googlesql::ResolvedOption>>&
        options,
    absl::string_view name) {
  for (const auto& opt : options) {
    if (opt == nullptr) continue;
    if (!absl::EqualsIgnoreCase(opt->name(), name)) continue;
    return OptionStringValue(opt.get());
  }
  return absl::NotFoundError(absl::StrCat(
      "control op executor: required option '", name, "' missing"));
}

absl::StatusOr<std::vector<std::string>> ExtractStringArrayLiteral(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: expected STRING array literal, got null expr");
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_LITERAL) {
    const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
    if (lit == nullptr) {
      return absl::InvalidArgumentError(
          "control op executor: uris literal is null");
    }
    const ::googlesql::Value& v = lit->value();
    if (v.type_kind() == ::googlesql::TYPE_ARRAY) {
      std::vector<std::string> out;
      out.reserve(v.num_elements());
      for (int i = 0; i < v.num_elements(); ++i) {
        const ::googlesql::Value& elem = v.element(i);
        if (elem.is_null() || elem.type_kind() != ::googlesql::TYPE_STRING) {
          return absl::InvalidArgumentError(
              "control op executor: uris array must contain STRING literals");
        }
        out.push_back(elem.string_value());
      }
      return out;
    }
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_FUNCTION_CALL) {
    const auto* call = expr->GetAs<::googlesql::ResolvedFunctionCall>();
    if (call == nullptr) {
      return absl::InvalidArgumentError(
          "control op executor: uris function call is null");
    }
    std::vector<std::string> out;
    out.reserve(call->argument_list_size());
    for (int i = 0; i < call->argument_list_size(); ++i) {
      auto elem = TryLiteralString(call->argument_list(i));
      if (!elem.has_value()) {
        return absl::InvalidArgumentError(
            "control op executor: uris array must contain STRING literals");
      }
      out.push_back(*elem);
    }
    if (!out.empty()) return out;
  }
  if (auto single = TryLiteralString(expr); single.has_value()) {
    return std::vector<std::string>{*single};
  }
  return absl::InvalidArgumentError(
      "control op executor: expected STRING array literal for uris");
}

absl::StatusOr<std::string> LocalPathFromUri(absl::string_view uri) {
  return LocalPathFromUri(uri, absl::string_view{});
}

absl::StatusOr<std::string> LocalPathFromUri(absl::string_view uri,
                                             absl::string_view data_dir) {
  if (uri.empty()) {
    return absl::InvalidArgumentError(
        "control op executor: URI must be non-empty");
  }
  if (absl::StartsWith(uri, "gs://")) {
    return MaterializeGCSObjectToCache(uri, data_dir);
  }
  if (absl::StartsWith(uri, "file://")) {
    return std::string(uri.substr(strlen("file://")));
  }
  return std::string(uri);
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
