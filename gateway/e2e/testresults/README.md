# query port emulator test logs

Run the ported `query_port_test.go` suite against `emulator_main`:

```bash
./gateway/e2e/testresults/run_query_port_tests.sh
```

Outputs (timestamped plus `*-latest.*` symlinks):

- `query-port-<UTC>.log` — human-readable verbose log (`-v` via JSON stream)
- `query-port-<UTC>.json` — machine-readable `go test -json` events
- `query-port-<UTC>-summary.txt` — pass/fail/skip counts and failure list

Requires `bin/emulator_main` (or `BIGQUERY_EMULATOR_BIN`).
