

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

namespace transpiler = ::bigquery_emulator::backend::engine::duckdb::transpiler;

// DML roots marked `semantic_executor` in `node_dispositions.yaml` still
// need a full AST walk: `VisitResolvedInsertStmt` promotes
// `INSERT ... SELECT` to DuckDB when the inner scan is transpilable, and
// `INSERT VALUES` falls back to the YAML disposition via `CheckNodeClass`.
bool DmlRootNeedsClassifierWalk(absl::string_view root_class) {
  return root_class == "ResolvedInsertStmt" ||
         root_class == "ResolvedUpdateStmt" ||
         root_class == "ResolvedDeleteStmt";
}

}  // namespace

RouteDecision RouteClassifier::Classify(
    const ::googlesql::ResolvedStatement& stmt) const {
  const std::string root_class =
      absl::StrCat("Resolved", stmt.node_kind_string());
  const auto* root_entry = transpiler::LookupNodeDisposition(root_class);

  if (root_entry != nullptr && !root_entry->planned) {
    if (root_entry->disposition == Disposition::kControlOp) {
      return RouteDecision{
          Disposition::kControlOp,
          RouteClassifierReasonFor(
              Disposition::kControlOp, root_class, root_class),
          root_class,
      };
    }
    if (root_entry->disposition == Disposition::kLocalStub) {
      return RouteDecision{
          Disposition::kLocalStub,
          RouteClassifierReasonFor(
              Disposition::kLocalStub, root_class, root_class),
          root_class,
      };
    }
    if (root_entry->disposition == Disposition::kSemanticExecutor) {
      if (!DmlRootNeedsClassifierWalk(root_class)) {
        return RouteDecision{
            Disposition::kSemanticExecutor,
            RouteClassifierReasonFor(
                Disposition::kSemanticExecutor, root_class, root_class),
            root_class,
        };
      }
    }
    if (root_entry->disposition == Disposition::kUnsupported) {
      return RouteDecision{
          Disposition::kUnsupported,
          RouteClassifierReasonFor(
              Disposition::kUnsupported, root_class, root_class),
          root_class,
      };
    }
  }

  RouteClassifierVisitor visitor;
  absl::Status walk = stmt.Accept(&visitor);
  if (!walk.ok()) {
    return RouteDecision{
        Disposition::kUnsupported,
        absl::StrCat("classifier visitor failed: ", walk.message()),
        root_class,
    };
  }

  Disposition d = visitor.disposition();
  std::string offending = visitor.offending_node();
  return RouteDecision{
      d,
      RouteClassifierReasonFor(d, root_class, offending),
      std::move(offending),
  };
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
