# `Emit*` empty-string return audit

Audit produced as item 1 of
`docs/ENGINE_POLICY.md`. Each row in
`backend/engine/duckdb/transpiler/transpiler.cc` that returns `""`
is categorized below. Readers should treat this as the source of
truth for "which `""` gates are defensive vs runtime and which are
caught by the classifier".

## Categories

| Category | Definition | Follow-up action |
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
  matching follow-up action is a classifier `VisitResolvedQueryStmt`
  that promotes to `kSemanticExecutor` when `is_value_table()`
  is true. Routes to the semantic executor; until that handler ships
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
  `docs/ENGINE_POLICY.md` /
  `docs/ENGINE_POLICY.md` (the semantic executor owns
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
  `docs/ENGINE_POLICY.md`. follow-up action:
  classifier `VisitResolvedArrayScan` promotes to
  `kSemanticExecutor` for any of these.
* **L573** `input_scan != nullptr && input_scan->node_kind != SingleRow`
  -- the lateral cross-join shape (`FROM t, UNNEST(t.arr)`).
  Same plan ownership; same follow-up action.
* **L576** -- propagation.

### `EmitAggregateScan` (L583-643)

* **L595** `if (node == nullptr)` -- defensive-null.
* **L599** `grouping_set_list_size > 0 || rollup_column_list_size > 0
  || grouping_call_list_size > 0` -- **child-class-gate**: the
  child rows (`ResolvedGroupingSet`, `ResolvedRollup`,
  `ResolvedCube`) already carry `duckdb_rewrite plan=
  docs/ENGINE_POLICY.md status=planned` in
  `node_dispositions.yaml`. The classifier visitor walks every
  child node, so the planned-row contract makes the route-promotion
  a no-op today (planned rows do not promote per the existing
  contract); the route stays at `kDuckdbNative` and the
  transpiler's `""` gate keeps the actual behavior at
  `UNIMPLEMENTED`. When `docs/ENGINE_POLICY.md`
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
  is `docs/ENGINE_POLICY.md`. **Deferred**: the
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
  COLLATE belongs to `docs/ENGINE_POLICY.md`
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
  belong to `docs/ENGINE_POLICY.md`. Deferred.

### `EmitSampleScan` (L1002-1075)

* **L1036** `node == nullptr || input_scan == nullptr` --
  defensive-null.
* **L1037** `repeatable_argument() != nullptr` -- **property-gate**.
  REPEATABLE seed semantics differ from DuckDB; owner is
  `docs/ENGINE_POLICY.md`. Deferred.
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
  `duckdb_native plan=docs/ENGINE_POLICY.md status=planned`
  in `node_dispositions.yaml`; the planned-row contract keeps the
  classifier from promoting and the transpiler returns `""` as
  the runtime UNIMPLEMENTED. When `docs/ENGINE_POLICY.md`
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
  `docs/ENGINE_POLICY.md`. **Deferred**.
* **propagation** (`if (a.empty()) return "";` per arg).
* **`status=planned` route surface** (`kDuckdbUdf` /
  `kSemanticExecutor` switch arms): see the
  `docs/ENGINE_POLICY.md` contract. The empty
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
  `docs/ENGINE_POLICY.md` (FORMAT clauses,
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
  docs/ENGINE_POLICY.md status=planned`; defensive
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

| `""` return category | Count | follow-up action |
|----------------------|-------|---------------|
| defensive-null | 32 | Keep as-is. |
| defensive-malformed | 22 | Keep as-is. |
| propagation | 31 | Keep as-is (the classifier walks the tree; the gate short-circuits a partial-emit in defense-in-depth depth). |
| property-gate (move to classifier in plan 2) | 4 | lands `VisitResolvedQueryStmt` (is_value_table) and `VisitResolvedJoinScan` (is_lateral). |
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
    `docs/ENGINE_POLICY.md` / `docs/ENGINE_POLICY.md`).

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
(`docs/ENGINE_POLICY.md`,
`docs/ENGINE_POLICY.md`,
`docs/ENGINE_POLICY.md`,
`docs/ENGINE_POLICY.md`). When each of those plans drops a
`status=planned` marker or ships its own classifier override, the
matching transpiler `""` gate becomes unreachable in practice
and gets removed in the same commit.

## Item 3 -- `functions.yaml` audit

Item 3 of the same plan asks: every `(planned)` row whose
underlying DuckDB target already matches BigQuery semantics flips
to `duckdb_native` / `duckdb_rewrite` (with a fixture in the same
commit, per "no silent approximation").

We walked every `status=planned` row in
`backend/engine/duckdb/transpiler/functions.yaml` (commit
`792278e`'s baseline). Each row's `notes` / inline comment
documents a SPECIFIC BigQuery semantic the DuckDB target does NOT
match:

| BQ function | Reason DuckDB target differs (paraphrased from row notes) | Owning plan |
|-------------|-----------------------------------------------------------|-------------|
| `log` (single + two-arg) | BQ `LOG(x)` == `LN(x)`; DuckDB `LOG(x)` == `LOG10(x)`. Two-arg `LOG(x, base)` needs a polyfill. | `docs/ENGINE_POLICY.md` |
| `mod` | BQ returns the second arg's type; DuckDB diverges on signed inputs. | `docs/ENGINE_POLICY.md` |
| `sqrt_numeric` | BQ `SQRT` over NUMERIC needs an explicit cast at the polyfill boundary. | `docs/ENGINE_POLICY.md` |
| `safe_divide` | BQ returns NULL on /0; DuckDB raises. SAFE semantics are exact. | `docs/ENGINE_POLICY.md` |
| `safe_negate` | BQ returns NULL on INT64 overflow; DuckDB raises. | `docs/ENGINE_POLICY.md` |
| `div` | BQ truncates toward zero; DuckDB `/` coerces to FLOAT. | `docs/ENGINE_POLICY.md` |
| `split`, `regexp_*`, `format`, `contains_substr`, `strpos`, `instr`, `soundex` | BQ RE2 dialect / `%`-FORMAT / locale rules differ from DuckDB regex / PRINTF. | `docs/ENGINE_POLICY.md` |
| `date_*`, `datetime_*`, `timestamp_*`, `extract`, `format_*`, `parse_*`, `unix_*` | Interval semantics, calendar-week / month-end arithmetic, format-string syntax all diverge. | `docs/ENGINE_POLICY.md` |
| `if` | BQ has CASE-of equivalence in DuckDB but corner cases differ; needs polyfill rewrite. | `docs/ENGINE_POLICY.md` |
| `isnull` | BQ has no `IS NULL` function form; the call-form path needs a UDF rewrite. | `docs/ENGINE_POLICY.md` |
| `countif` | BQ `COUNTIF(b)` lowers to DuckDB `COUNT(*) FILTER (WHERE b)`; needs structural rewrite. | `docs/ENGINE_POLICY.md` |

**Migration list: zero rows.** Every `(planned)` row carries a
documented edge-case BigQuery cares about that DuckDB does NOT
match without a structural rewrite (`duckdb_rewrite`) or a
polyfill UDF (`duckdb_udf`). Per "no silent approximation" we do
NOT promote any of these rows in plan 2; the polyfill plan
(`docs/ENGINE_POLICY.md`) and the
semantic-functions plan
(`docs/ENGINE_POLICY.md`) own the migration when
they ship.

The Done-Criterion 3 (`functions.yaml` has no `(planned)` row
whose DuckDB target already matches BigQuery semantics; every
remaining non-`duckdb_*` row carries a planned route from one of
the new-route plans named in the row's `plan=` field) is
therefore satisfied today as the baseline -- no plan-2 changes to
`functions.yaml` were needed.

## Item 3 -- update from the polyfill UDF library landing

`docs/ENGINE_POLICY.md` shipped the
first set of `duckdb_udf` wrappers. The following `functions.yaml`
rows flipped from `status=planned duckdb_udf` to ready
`duckdb_udf` and now point at a registered macro in
`backend/engine/duckdb/udf/`:

| BQ function | Macro (in `backend/engine/duckdb/udf/`) | BigQuery edge case the macro closes | Unit test that pins it |
|-------------|-----------------------------------------|-------------------------------------|------------------------|
| `mod`       | `numeric/numeric_macros.cc::bq_mod`     | Sign-of-dividend on negatives; raises on Y=0 | `numeric_macros_test::ModSignTracksDividend`, `ModByZeroRaises` |
| `div`       | `numeric/numeric_macros.cc::bq_div`     | Truncated (not floor) integer division; raises on Y=0 | `numeric_macros_test::DivTruncatesNotFloors`, `DivByZeroRaises` |
| `if`        | `conditional/conditional_macros.cc::bq_if` | NULL cond falls through to ELSE (not the THEN branch) | `conditional_macros_test::IfNullCondFallsThroughToElse` |
| `isnull`    | `conditional/conditional_macros.cc::bq_isnull` | Empty string is NOT NULL in BigQuery | `conditional_macros_test::IsNullOnNonNullValues` |
| `strpos`    | `string/string_macros.cc::bq_strpos`    | 1-based index; missing needle returns 0 (not NULL); empty needle returns 1 | `string_macros_test::StrposReturnsOneBasedIndex`, `StrposMissingNeedleReturnsZero`, `StrposEmptyNeedle` |
| `countif`   | n/a -- routed `duckdb_native duckdb_name=count_if` (no macro layer; DuckDB v1.5.3's `count_if` matches BQ COUNTIF on NULL / FALSE handling) | NULL inputs are NOT counted (treated as FALSE-like) | `conformance/fixtures/functions/aggregate/function_countif.yaml` (NULL row excluded from the `true_count`) |
| `log`       | `numeric/numeric_macros.cc::bq_log` (variadic: `bq_log(x)` natural-log, `bq_log(x, base)` base-second per BQ argument order) | `LOG(x)` is BASE-e (not base-10 like DuckDB's bare `log`); `LOG(x, base)` argument order is value-first (DuckDB's `log(b, x)` is base-first) | `numeric_macros_test::LogSingleArgIsNaturalLog`, `LogTwoArgIdentity`, `LogNullPropagation` |
| `regexp_contains` | `regex/regex_macros.cc::bq_regexp_contains` (thin alias around DuckDB `regexp_matches`; both engines vendor RE2) | RE2 dialect compatibility (anchoring, case-sensitive by default, `(?i)` inline flags) -- the macro pins our own name so a future DuckDB upgrade swapping regex engines would surface as a unit-test failure | `regex_macros_test::RegexpContainsAnchoredMatch`, `RegexpContainsCaseSensitiveByDefault`, `RegexpContainsHonorsInlineFlags`, `RegexpContainsNullPropagation` |
| `regexp_replace` | `regex/regex_macros.cc::bq_regexp_replace` (forwards `'g'` to DuckDB's `regexp_replace`) | BigQuery replaces ALL non-overlapping matches; DuckDB defaults to FIRST-only. The macro hardcodes the global flag so the BQ contract holds at the call site | `regex_macros_test::RegexpReplaceIsGlobal`, `RegexpReplaceHonorsBackreferences`, `RegexpReplaceNullPropagation` |
| `split`     | `string/string_macros.cc::bq_split` (DEFAULT delimiter `,` via `delimiter := ','`) | BigQuery SPLIT(value) defaults to splitting on `,`; DuckDB's `string_split` requires both arguments | `string_macros_test::SplitDefaultDelimiterIsComma`, `SplitCustomDelimiter`, `SplitEmptyInputReturnsSingleEmpty`, `SplitNullPropagation` |
| `unix_seconds` | `datetime/datetime_macros.cc::bq_unix_seconds` (`CAST(FLOOR(epoch(t)) AS BIGINT)`) | BigQuery UNIX_SECONDS "rounds down to the beginning of the second"; DuckDB's bare `CAST(epoch(t) AS BIGINT)` ROUNDS (so .999 becomes +1). The explicit FLOOR wrap pins the round-down contract | `datetime_macros_test::UnixSecondsTruncatesSubsecond`, `UnixSecondsWholeSecond`, `UnixSecondsNullPropagation` |
| `unix_millis` | `datetime/datetime_macros.cc::bq_unix_millis` (thin alias around DuckDB's BIGINT-returning `epoch_ms`) | Pins the BIGINT-millis contract under our own name so a future DuckDB precision drift would surface as a unit-test failure | `datetime_macros_test::UnixMillisWholeSecond`, `UnixMillisSubsecond`, `UnixMillisNullPropagation` |
| `unix_micros` | `datetime/datetime_macros.cc::bq_unix_micros` (thin alias around DuckDB's `epoch_us`) | Same as `unix_millis` -- thin alias under our name | `datetime_macros_test::UnixMicrosWholeSecond`, `UnixMicrosSubsecond`, `UnixMicrosNullPropagation` |
| `unix_date` | `datetime/datetime_macros.cc::bq_unix_date` (`date_diff('day', '1970-01-01'::DATE, d)`) | `date_diff` is calendar-day-boundary counted, unambiguous on DST-shift days. Pre-epoch dates return negative day counts (BigQuery documents this explicitly) | `datetime_macros_test::UnixDateEpochIsZero`, `UnixDatePreEpochIsNegative`, `UnixDateFutureDate`, `UnixDateNullPropagation` |

The following rows were investigated during the polyfill landing
and found to require more than a thin DuckDB macro; they
re-pointed at `docs/ENGINE_POLICY.md` (still
`status=planned`):

| BQ function | Reason the gap is wider than a thin macro |
|-------------|--------------------------------------------|
| `contains_substr` | BigQuery applies Unicode NFKC + case-folding before substring search; DuckDB's `contains` is exact byte-level match and DuckDB v1.5.3 ships no NFKC primitive. |
| `instr` | BigQuery INSTR is variadic with negative-position semantics; DuckDB's `instr` is 2-arg only. The variadic surface is more naturally implemented in Go. |
| `soundex` | DuckDB v1.5.3 does not ship a `soundex` scalar function (verified: `SELECT soundex('Robert')` -> Catalog Error). |
| `sqrt_numeric` | Placeholder row for the case where BigQuery SQRT is called with a NUMERIC argument and the transpiler needs to insert an explicit cast. The current emit path looks up by lowercase function NAME only and never lowers anything to `sqrt_numeric` (the analyzer resolves to `sqrt`). Closing the gap requires signature-aware function dispatch in the transpiler -- a transpiler architecture change, not a thin polyfill macro. |
| `regexp_extract`, `regexp_extract_all` | BigQuery returns the FIRST capturing group when the regex contains one and falls back to the whole match otherwise; DuckDB always returns the whole match by default and exposes the group via a separate numeric `group` argument. A thin macro cannot introspect the regex pattern to choose the right behavior. The discrimination is a few lines of Go in the semantic executor (parse the regex, count `(...)` groups minus non-capturing `(?:...)`, dispatch to `regexp_extract(..., 1)` or `regexp_extract(..., 0)`). |
| `format` | Printf-style with BigQuery extensions (`%t` / `%T` type-aware rendering, `%p` parameterized substitution, type-specific ARRAY / STRUCT / JSON format codes, `%E*` extended-year extensions). DuckDB's `printf` / `format` implement only a subset of POSIX printf. The format-spec translation table is more naturally expressed in Go. |
| `date_add`, `date_sub`, `datetime_add`, `datetime_sub`, `timestamp_add`, `timestamp_sub` | BigQuery's MONTH-END SNAP: `DATE_ADD(DATE '2024-01-31', INTERVAL 1 MONTH) == DATE '2024-02-29'`. DuckDB overflows into the next month. Same snap applies to YEAR additions on Feb 29. Closing the gap requires a CASE-arm rewrite plus timezone-aware variants and is more naturally expressed in Go. |
| `date_diff`, `datetime_diff`, `timestamp_diff` | BigQuery counts CALENDAR BOUNDARIES (e.g. `DATE_DIFF(DATE '2024-01-31', DATE '2024-01-01', MONTH) == 0` because no calendar-month boundary was crossed). DuckDB's `date_diff` is elapsed-units based. WEEK parts also have a parameterizable start-of-week. The semantic executor owns the rewrite. |
| `date_trunc`, `datetime_trunc`, `timestamp_trunc` | Calendar-week / ISOWEEK / ISOYEAR / QUARTER alignment differs; TIMESTAMP_TRUNC takes an explicit timezone argument that DuckDB's `date_trunc` doesn't accept. |
| `extract` | BigQuery-specific parts (DATE / TIME / DATETIME extraction with TZ, ISOYEAR, ISOWEEK, MICROSECOND, DAYOFYEAR / DAYOFWEEK with Sunday-is-1 convention) diverge from DuckDB's ISO-8601-only `extract`. |
| `format_timestamp`, `format_date`, `format_datetime`, `parse_timestamp`, `parse_date`, `parse_datetime` | BigQuery uses extended strftime (`%E4Y`, `%E*S`, `%Q`, `%Ez`, `%Z`) on top of POSIX strftime. DuckDB's `strftime` / `strptime` implement the POSIX core but not the `%E*` / `%Q` / `%Ez` / `%Z` extensions. The format-spec translation table is extensive and more naturally expressed in Go. |

Net result: **zero `status=planned duckdb_udf` rows** remain in
`functions.yaml` after this plan landing -- every former
`status=planned duckdb_udf` row either flipped to ready
`duckdb_udf` / `duckdb_native` with a wrapper + unit test +
conformance fixture, or re-pointed at
`status=planned semantic_executor plan=docs/ENGINE_POLICY.md`
with a documented BigQuery / DuckDB gap that no thin macro can
close. The polyfill plan is no longer the owning plan for any
remaining gap; the semantic-functions plan picks up where this
plan left off.
