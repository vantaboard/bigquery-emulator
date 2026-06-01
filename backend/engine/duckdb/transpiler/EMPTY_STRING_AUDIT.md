# `Emit*` empty-string return audit

Audit produced as item 1 of
`.cursor/plans/duckdb-fast-path-stabilization.plan.md`. Each row in
`backend/engine/duckdb/transpiler/transpiler.cc` that returns `""`
is categorized below. Readers should treat this as the source of
truth for "which `""` gates are defensive vs runtime and which are
caught by the classifier".

## Categories

| Category | Definition | Plan-2 action |
|----------|------------|---------------|
| **defensive-null** | `nullptr` checks on the resolved node, its `type()`, an inner expression, etc. The analyzer guarantees these are non-null for a well-formed AST; the gate exists to harden a misuse-from-test or a malformed serialized AST round-trip. | Keep. These are not runtime gates. |
| **defensive-malformed** | Guards that catch an analyzer-contract violation (e.g. `column_index_list_size != column_list_size`, `field_idx out of range`, `default:` arm of an exhaustive `switch`). | Keep. These are not runtime gates. |
| **propagation** | `if (sub_emit.empty()) return "";` after a recursive `Emit*` call. The classifier catches the underlying root cause via tree walk; this gate exists to short-circuit a partially-emitted SQL fragment. | Keep. These are not runtime gates per se -- they propagate. |
| **property-gate** | Bails on a NODE PROPERTY (boolean / list-size / accessor) that the analyzer set on a node kind whose YAML row classifies as `duckdb_*`. The classifier visitor today does NOT see the property, only the node kind, so removing the gate here without adding a `Visit*` override on the classifier would let the transpiler emit invalid SQL. | Move into a classifier `Visit*` override. |
| **child-class-gate** | Bails on the presence of a CHILD NODE whose class disposition differs from the wrapping node's. The classifier already catches these via the tree walk + the YAML row for the child class; the gate is structurally a defensive duplicate. | Keep as defense-in-depth (cheap), but the gate is unreachable in practice once the classifier is in front. |

## Per-method audit

### Single-entry dispatchers

`Transpile()` (line 188-218), `EmitExpr()` (220-245), `EmitScan()`
(247-281): every `""` return here is either `node == nullptr`
(defensive-null) or the `default:` arm of a node-kind switch
(defensive-malformed). The default arms are reachable only when a
new resolved-AST class lands without a row in
`node_dispositions.yaml` — the parity checker (`task
lint:dispositions`) catches that before main.

* **L188** `if (node == nullptr) return "";` -- defensive-null.
* **L216** `default: return "";` (Transpile) -- defensive-malformed.
* **L221** `if (expr == nullptr) return "";` -- defensive-null.
* **L243** `default: return "";` (EmitExpr) -- defensive-malformed.
* **L248** `if (scan == nullptr) return "";` -- defensive-null.
* **L279** `default: return "";` (EmitScan) -- defensive-malformed.

### `EmitValueLiteral` (file-private, L106-139)

* **L109** `if (type == nullptr) return "";` -- defensive-null.
* **L118** array element propagation -- propagation.
* **L125** `st == nullptr || st->num_fields() != v.num_fields()` --
  defensive-malformed (analyzer contract: STRUCT type / value
  field counts agree).
* **L130** struct field propagation -- propagation.

### `EmitQueryStmt` (L289-316)

* **L302** `if (node == nullptr) return "";` -- defensive-null.
* **L303** `if (node->is_value_table()) return "";` --
  **PROPERTY-GATE.** `SELECT AS VALUE ...` collapses the row to a
  single anonymous value; DuckDB has no direct analog. The
  matching plan-2 action is a classifier `VisitResolvedQueryStmt`
  that promotes to `kSemanticExecutor` when `is_value_table()`
  is true. The wrapping plan is the semantic executor itself
  (`semantic-executor-core.plan.md`); until that plan ships
  the stub `SemanticExecutor` returns `UNIMPLEMENTED`, which is
  the same end-user-visible behavior as the empty-string gate.
* **L305, L310, L313** -- propagation.

### `EmitProjectScan` (L320-400)

* **L349** `if (node == nullptr) return "";` -- defensive-null.
* **L374, L390** -- propagation.

### `EmitTableScan` (L402-443)

* **L411** `if (node == nullptr || node->table() == nullptr)` --
  defensive-null.
* **L423** `if (i >= node->column_index_list_size())` --
  defensive-malformed (analyzer contract: `column_list` and
  `column_index_list` are 1:1).
* **L426** `if (src == nullptr)` -- defensive-malformed.

### `EmitSingleRowScan` (L445-458)

* **L456** `if (node == nullptr)` -- defensive-null.

### `EmitFilterScan` (L460-476)

* **L470** `if (node == nullptr)` -- defensive-null.
* **L472, L474** -- propagation.

### `EmitJoinScan` (L478-534)

* **L491** `if (node == nullptr)` -- defensive-null.
* **L494** `is_lateral() || has_using() || parameter_list_size > 0`
  -- **PROPERTY-GATE.** Lateral joins (`is_lateral`) and
  lateral-correlated `parameter_list` slots both belong to
  `array-struct-semantic-path.plan.md` /
  `cte-subquery-routing.plan.md` (the semantic executor owns
  evaluation order for those). `has_using()` is the analyzer's
  marker for `JOIN ... USING(...)`; the analyzer canonicalizes
  USING into ON, but the column-list collapse rule needs a
  bespoke rewrite the first emit pass does not cover. Plan-2
  action: classifier `VisitResolvedJoinScan` promotes to
  `kSemanticExecutor` for any of these three, and the
  transpiler removes the gate (defensive-only fallback in
  case the classifier is bypassed).
* **L497, L499** -- propagation.
* **L516** `default: return "";` on `join_type` -- defensive-malformed.
* **L526** non-INNER + `join_expr == nullptr` -- defensive-malformed
  (a grammar error the analyzer rejects upstream).
* **L531** -- propagation.

### `EmitArrayScan` (L536-581)

* **L560** `if (node == nullptr)` -- defensive-null.
* **L565** `array_expr_list_size != 1 || element_column_list_size != 1
  || array_offset_column != nullptr || join_expr != nullptr ||
  is_outer || array_zip_mode != nullptr` --
  **PROPERTY-GATE.** Multi-array UNNEST (`array_zip_mode`),
  `WITH OFFSET` (`array_offset_column`), LEFT/RIGHT outer UNNEST
  (`is_outer` / `join_expr`) all belong to
  `array-struct-semantic-path.plan.md`. Plan-2 action:
  classifier `VisitResolvedArrayScan` promotes to
  `kSemanticExecutor` for any of these.
* **L573** `input_scan != nullptr && input_scan->node_kind != SingleRow`
  -- the lateral cross-join shape (`FROM t, UNNEST(t.arr)`).
  Same plan ownership; same plan-2 action.
* **L576** -- propagation.

### `EmitAggregateScan` (L583-643)

* **L595** `if (node == nullptr)` -- defensive-null.
* **L599** `grouping_set_list_size > 0 || rollup_column_list_size > 0
  || grouping_call_list_size > 0` -- **child-class-gate**: the
  child rows (`ResolvedGroupingSet`, `ResolvedRollup`,
  `ResolvedCube`) already carry `duckdb_rewrite plan=
  advanced-relational-routing.plan.md status=planned` in
  `node_dispositions.yaml`. The classifier visitor walks every
  child node, so the planned-row contract makes the route-promotion
  a no-op today (planned rows do not promote per the existing
  contract); the route stays at `kDuckdbNative` and the
  transpiler's `""` gate keeps the actual behavior at
  `UNIMPLEMENTED`. When `advanced-relational-routing.plan.md`
  drops the `status=planned` marker, the classifier will start
  promoting and this gate becomes unreachable in practice. Keep
  as defense-in-depth.
* **L602, L611, L613, L622, L627, L631** -- propagation /
  defensive-malformed.

### `EmitSetOperationItem` (L645-685) and `EmitSetOperationScan`
(L687-769)

* **L662** `item == nullptr || parent == nullptr || item->scan() == nullptr`
  -- defensive-null.
* **L665** `output_column_list_size != parent->column_list_size()` --
  defensive-malformed.
* **L668** -- propagation.
* **L724** `if (node == nullptr)` -- defensive-null.
* **L727** `column_match_mode != BY_POSITION` -- **property-gate**.
  CORRESPONDING / CORRESPONDING_BY needs a name-based reshuffle
  the positional projection does not handle. The wrapping shape
  is still `ResolvedSetOperationScan` with disposition
  `duckdb_native`; plan-2 owner for the classifier promotion
  is `advanced-relational-routing.plan.md`. **Deferred**: the
  CORRESPONDING surface is BigQuery-only and covered by the
  advanced relational plan; the property-gate stays in the
  transpiler today and we promote in plan 12.
* **L735** `input_item_list_size < 2` -- defensive-malformed.
* **L758** `default: return "";` on `op_type` -- defensive-malformed.
* **L765** -- propagation.

### `EmitOrderByScan` (L771-811)

* **L782** `if (node == nullptr)` -- defensive-null.
* **L784** -- propagation.
* **L789** `item == nullptr || item->column_ref() == nullptr` --
  defensive-null.
* **L790** `item->collation_name() != nullptr` -- **property-gate**.
  COLLATE belongs to `semantic-functions-compliance.plan.md`
  (collation-aware comparison). Deferred: same reasoning as
  CORRESPONDING above.
* **L792, L808** -- propagation.

### Window-clause builders (L860-948)

* `BuildPartitionClause` (L860-877), `BuildOrderClause`
  (L879-896), `BuildFrameClause` (L898-917),
  `BuildAnalyticProjection` (L919-948), `EmitAnalyticScan`
  (L950-1000): every `""` here is either defensive-null or
  propagation, except the property-gates against
  `collation_list` (L865) and `hint_list` (L865, L882) which
  belong to `semantic-functions-compliance.plan.md`. Deferred.

### `EmitSampleScan` (L1002-1075)

* **L1036** `node == nullptr || input_scan == nullptr` --
  defensive-null.
* **L1037** `repeatable_argument() != nullptr` -- **property-gate**.
  REPEATABLE seed semantics differ from DuckDB; owner is
  `advanced-relational-routing.plan.md`. Deferred.
* **L1038** `weight_column() != nullptr` -- **property-gate**. WITH
  WEIGHT has no DuckDB analog. Deferred (same plan).
* **L1039** `partition_by_list_size > 0` -- **property-gate**.
  STRATIFY BY has no DuckDB analog. Deferred (same plan).
* **L1045** unknown method -- defensive-malformed.
* **L1057, L1062, L1066** -- defensive-malformed (method/unit
  matrix that DuckDB rejects at parse time).

### `EmitWithRefScan` (L1077-1080)

* **L1079** `return "";` -- placeholder for the planned CTE-ref
  emit. The wrapping `ResolvedWithRefScan` row already carries
  `duckdb_native plan=cte-subquery-routing.plan.md status=planned`
  in `node_dispositions.yaml`; the planned-row contract keeps the
  classifier from promoting and the transpiler returns `""` as
  the runtime UNIMPLEMENTED. When `cte-subquery-routing.plan.md`
  drops the `planned` marker, the placeholder body lands too.
  **Defensive** (deferred-by-design).

### `EmitParameter` (L1104-1132)

* **L1121** `if (node == nullptr)` -- defensive-null.
* **L1122** `is_untyped()` -- defensive-malformed (an analyzer
  configuration where the parameter type is filled in at bind
  time; the transpiled-query path does not use that mode).
* **L1131** `name="" && position<=0` -- defensive-malformed.

### `EmitColumnRef` (L1151-1160)

* **L1158** `if (node == nullptr)` -- defensive-null.

### `EmitFunctionCall` / `EmitAggregateFunctionCall` /
`EmitAnalyticFunctionCall` (L1162-1381)

Every `""` here is one of:

* **defensive-null** (`node == nullptr || function == nullptr`,
  `argument == nullptr`).
* **property-gate** (`SAFE_ERROR_MODE`, aggregate modifiers
  HAVING/ORDER BY/LIMIT/GROUP BY/NULL-handling, analytic
  IGNORE/RESPECT NULLS). All belong to
  `semantic-functions-compliance.plan.md`. **Deferred**.
* **propagation** (`if (a.empty()) return "";` per arg).
* **`status=planned` route surface** (`kDuckdbUdf` /
  `kSemanticExecutor` switch arms): see the
  `execution-disposition-registry.plan.md` contract. The empty
  string here mirrors the per-row `status=planned`; once the
  matching plan ships, the row's marker drops and the case stops
  returning `""`.
* **`kUnsupported` switch arm**: this is the route-table case
  for `unsupported` functions (`approx_quantiles`, `ml.*`,
  `net.*`, ...). The classifier already routes these via
  `VisitResolvedFunctionCall` -- the transpiler is unreachable
  for unsupported-route queries today. **Defensive**
  (defense-in-depth).

### `EmitFrameBound` (L1383-1407)

* **L1385** `if (expr == nullptr)` -- defensive-null.
* **L1394, L1396, L1400, L1402** -- propagation /
  defensive-malformed.
* **L1406** `return "";` (final fallthrough, unreachable) --
  defensive-malformed.

### `EmitCast` (L1409-1448)

* **L1435** `node == nullptr || expr == nullptr` -- defensive-null.
* **L1436-L1438** `format() / time_zone() / extended_cast() /
  type_modifiers()` -- **property-gate**. Owner is
  `semantic-functions-compliance.plan.md` (FORMAT clauses,
  collation modifiers, time-zone-aware casts). Deferred.
* **L1440** `target == nullptr` -- defensive-null.
* **L1441** `IsCastTargetSupported(kind) == false` --
  **property-gate** but for an unsupported route
  (GEOGRAPHY / proto / enum / range / graph / measure /
  tokenlist all carry `unsupported` in the registry). Deferred:
  the classifier needs a `VisitResolvedCast` override to read
  the cast target type and promote the unsupported case.
  Today the wrapping `ResolvedCast` row is `duckdb_native`, so
  the classifier walk does not see the unsupported route until
  the cast lowers. The gate is the runtime UNIMPLEMENTED.
* **L1443, L1445** -- propagation.

### `EmitMakeStruct` (L1450-1492)

* **L1476** `node == nullptr` -- defensive-null.
* **L1478** `t == nullptr || !t->IsStruct()` -- defensive-malformed.
* **L1481** `st == nullptr || num_fields != field_list_size` --
  defensive-malformed.
* **L1487** -- propagation.

### `EmitGetStructField` (L1494-1524)

* **L1509** `node == nullptr || expr == nullptr` -- defensive-null.
* **L1511** `base_type == nullptr || !base_type->IsStruct()` --
  defensive-malformed.
* **L1513** `st == nullptr` -- defensive-null.
* **L1515** `idx < 0 || idx >= num_fields` -- defensive-malformed.
* **L1517** -- propagation.

### `EmitGetJsonField` (L1526-1570)

* **L1562** `node == nullptr || expr == nullptr` -- defensive-null.
* **L1564** -- propagation.

### `EmitSubqueryExpr` (L1572-1575)

* **L1574** `return "";` -- placeholder for the planned subquery
  emit, mirroring `EmitWithRefScan` above. Wrapping row
  `ResolvedSubqueryExpr` is `duckdb_native plan=
  cte-subquery-routing.plan.md status=planned`; defensive
  (deferred-by-design).

### `EmitWithExpr` (L1577-1616)

* **L1597** `node == nullptr || expr == nullptr` -- defensive-null.
* **L1598** `assignment_list_size == 0` -- defensive-malformed.
* **L1603, L1605, L1613** -- propagation.

### `EmitFunctionArgument` (L1618-1650)

* **L1636, L1637** -- defensive-null.
* **L1642** `scan / model / connection / descriptor / inline_lambda
  / sequence / graph slots set` -- **child-class-gate**: each
  of those slots maps to a child node class
  (`ResolvedTVFScan`, `ResolvedInlineLambda`,
  `ResolvedRelationArgumentScan`, `ResolvedSequence`, ...) whose
  YAML row classifies away from `duckdb_native`. The classifier
  walk catches these. Defense-in-depth.

### `EmitOutputColumn` / `EmitComputedColumn` (L1654-1684)

* **L1662, L1680** -- defensive-null.
* **L1682** -- propagation.

## Summary table

| `""` return category | Count | Plan-2 action |
|----------------------|-------|---------------|
| defensive-null | 32 | Keep as-is. |
| defensive-malformed | 22 | Keep as-is. |
| propagation | 31 | Keep as-is (the classifier walks the tree; the gate short-circuits a partial-emit in defense-in-depth depth). |
| property-gate (move to classifier in plan 2) | 4 | Plan-2 lands `VisitResolvedQueryStmt` (is_value_table) and `VisitResolvedJoinScan` (is_lateral). |
| property-gate (deferred to a later plan) | ~14 | Documented per-method above with the owning plan. |
| child-class-gate | 2 | Keep as defense-in-depth (the classifier already catches via tree walk). |
| `status=planned` placeholder body | 2 | Keep as-is; the row's `planned` marker drives the runtime UNIMPLEMENTED. |

## What plan 2 lands in C++

* **Classifier** (`backend/engine/coordinator/route_classifier.cc`):
  * `VisitResolvedQueryStmt`: promote to `kSemanticExecutor` when
    `is_value_table()` is true.
  * `VisitResolvedJoinScan`: promote to `kSemanticExecutor` when
    `is_lateral()` is true (the lateral case is the most narrowly
    semantic-executor-bound of the three flags; HAS USING / lateral
    parameter list stay in the deferred queue and are picked up by
    `array-struct-semantic-path.plan.md` / `cte-subquery-routing.plan.md`).

* **Transpiler** (`backend/engine/duckdb/transpiler/transpiler.cc`):
  * Drop the `is_value_table()` early-return in `EmitQueryStmt`
    (now caught by the classifier).
  * Drop the `is_lateral()` clause from `EmitJoinScan`'s
    early-return (now caught by the classifier). The
    `has_using() || parameter_list_size > 0` clause stays in
    place per the deferred queue above.

* **Tests**: `route_classifier_test` gains two cases, one per
  promotion. The transpiler keeps its existing
  `transpiler_test` shape coverage; the dropped early-returns are
  unreachable in practice.

The deferred property-gates land via their owning plans
(`array-struct-semantic-path.plan.md`,
`semantic-functions-compliance.plan.md`,
`advanced-relational-routing.plan.md`,
`cte-subquery-routing.plan.md`). When each of those plans drops a
`status=planned` marker or ships its own classifier override, the
matching transpiler `""` gate becomes unreachable in practice
and gets removed in the same commit.
