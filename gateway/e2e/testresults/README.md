# googlesqlite emulator port test logs

Run the ported `googlesqlite/query_test.go` suite against `emulator_main`:

```bash
./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh
```

Outputs (timestamped plus `*-latest.*` symlinks):

- `googlesqlite-emulator-<UTC>.log` — human-readable verbose log (`-v` via JSON stream)
- `googlesqlite-emulator-<UTC>.json` — machine-readable `go test -json` events
- `googlesqlite-emulator-<UTC>-summary.txt` — pass/fail/skip counts and failure list

Requires `bin/emulator_main` (or `BIGQUERY_EMULATOR_BIN`).
