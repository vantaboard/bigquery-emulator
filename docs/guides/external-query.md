# EXTERNAL_QUERY (fixture-backed)

The emulator does **not** proxy `EXTERNAL_QUERY` to live Cloud SQL, Spanner, or
AlloyDB. Federated queries resolve from committed snapshots under:

```text
$data_dir/external/connections/<connection_id>/
  queries.yaml    # or queries.json
  <result>.json   # schema + rows per manifest entry
```

## Manifest (`queries.yaml`)

```yaml
queries:
  - query: "SELECT id, name FROM users ORDER BY id"
    result: users.json
  - alias: info_schema_tables
    result: info_schema.json
```

Each `query` value must match the **exact** SQL string passed as the second
argument to `EXTERNAL_QUERY`. Alternatively, use a short `alias` when the
fixture author prefers stable names.

## Result file (`users.json`)

Uses the same typed-cell vocabulary as conformance fixtures:

```json
{
  "schema": [
    {"name": "id", "type": "INT64"},
    {"name": "name", "type": "STRING"}
  ],
  "rows": [
    {"id": 1, "name": "ada"},
    {"id": 2, "name": "linus"}
  ]
}
```

## Query example

```sql
SELECT *
FROM EXTERNAL_QUERY(
  'us.my_conn',
  'SELECT id, name FROM users ORDER BY id'
)
```

The connection argument may be `region.connection_id` (for example `us.my_conn`)
or a full resource name; only the final segment selects the fixture directory.

## Missing fixtures

When no manifest or query entry exists, analysis/execution returns `NOT_FOUND`
with a message naming `$data_dir/external/connections/<id>/` so operators know
where to add snapshots.

## Connection API

`google.cloud.bigquery.connection.v1.ConnectionService` persists connection
metadata under `$data_dir/external/connections/_registry/connections.json`.
Property blocks (`cloudSql`, `cloudSpanner`, …) round-trip for client startup
probes; no live credentials or federation is performed.

## Mode resolution

Default mode is `fixture` (see `gateway/external/sourceconfig/`). Override per
connection id in `$data_dir/external_sources.yaml` or with
`BIGQUERY_EMULATOR_EXTERNAL_CONNECTIONS_MODE`.

See also `docs/ENGINE_POLICY.md` (External query and federated sources).
