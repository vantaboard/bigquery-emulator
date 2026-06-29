#include "backend/engine/semantic/outer_row_eval.h"

#include <string>
#include <utility>

#include "absl/strings/str_cat.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

const ::googlesql::Value* LookupOuterValue(
    const ColumnBindings& outer_row,
    const absl::flat_hash_map<std::string, ::googlesql::Value>* outer_by_name,
    const ::googlesql::ResolvedColumn& col) {
  auto it = outer_row.find(col.column_id());
  if (it != outer_row.end()) return &it->second;
  if (outer_by_name == nullptr) return nullptr;
  const std::string qualified = absl::StrCat(col.table_name(), ".", col.name());
  auto nit = outer_by_name->find(qualified);
  if (nit != outer_by_name->end()) return &nit->second;
  nit = outer_by_name->find(std::string(col.name()));
  if (nit != outer_by_name->end()) return &nit->second;
  return nullptr;
}

}  // namespace

OuterRowFrame MakeOuterRowFrame(
    const EvalContext& parent_ctx,
    const ColumnBindings& outer_row,
    const ::googlesql::ResolvedScan* scan_for_names) {
  OuterRowFrame frame;
  if (parent_ctx.columns != nullptr) {
    frame.merged = *parent_ctx.columns;
  }
  for (const auto& [col_id, val] : outer_row) {
    frame.merged[col_id] = val;
  }
  if (parent_ctx.columns_by_name != nullptr) {
    for (const auto& [name, val] : *parent_ctx.columns_by_name) {
      frame.by_name[name] = val;
    }
  }
  if (scan_for_names != nullptr) {
    for (int i = 0; i < scan_for_names->column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& col = scan_for_names->column_list(i);
      auto it = frame.merged.find(col.column_id());
      if (it == frame.merged.end()) continue;
      frame.by_name[std::string(col.name())] = it->second;
      frame.by_name[absl::StrCat(col.table_name(), ".", col.name())] =
          it->second;
    }
  }
  frame.row_ctx = parent_ctx;
  frame.row_ctx.columns = &frame.merged;
  frame.row_ctx.columns_by_name = &frame.by_name;
  return frame;
}

void BindCorrelatedColumnRefs(const ::googlesql::ResolvedScan* correlated_scan,
                              OuterRowFrame& frame) {
  if (correlated_scan == nullptr) return;

  struct CorrelatedBinder : public ::googlesql::ResolvedASTVisitor {
    const absl::flat_hash_map<std::string, ::googlesql::Value>& outer_by_name;
    const ColumnBindings& outer_by_id;
    ColumnBindings& bind;
    absl::flat_hash_map<std::string, ::googlesql::Value>& by_name;

    CorrelatedBinder(
        const absl::flat_hash_map<std::string, ::googlesql::Value>& outer_in,
        const ColumnBindings& outer_id_in,
        ColumnBindings& bind_in,
        absl::flat_hash_map<std::string, ::googlesql::Value>& by_name_in)
        : outer_by_name(outer_in),
          outer_by_id(outer_id_in),
          bind(bind_in),
          by_name(by_name_in) {}

    absl::Status DefaultVisit(const ::googlesql::ResolvedNode* n) override {
      return ::googlesql::ResolvedASTVisitor::DefaultVisit(n);
    }

    absl::Status VisitResolvedColumnRef(
        const ::googlesql::ResolvedColumnRef* ref) override {
      if (ref == nullptr) {
        return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
      }
      const int col_id = ref->column().column_id();
      if (bind.contains(col_id)) {
        return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
      }
      const ::googlesql::Value* val =
          LookupOuterValue(outer_by_id, &outer_by_name, ref->column());
      if (val == nullptr) {
        return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
      }
      bind.emplace(col_id, *val);
      const std::string qualified =
          absl::StrCat(ref->column().table_name(), ".", ref->column().name());
      by_name[qualified] = *val;
      by_name[std::string(ref->column().name())] = *val;
      return ::googlesql::ResolvedASTVisitor::VisitResolvedColumnRef(ref);
    }
  };

  CorrelatedBinder binder{
      frame.by_name, frame.merged, frame.merged, frame.by_name};
  (void)correlated_scan->Accept(&binder);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
