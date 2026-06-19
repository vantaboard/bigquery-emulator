---
name: M3-01 — Create Table modal
overview: Build the Create table modal opened from the dataset page, with a Source selector (all eight create-from options), Destination fields, a full schema builder (visual + edit-as-text, nested/repeated/range types), partitioning/clustering/tags/advanced options, and format-specific upload options. Submits via the appropriate insert/load/DDL path.
todos:
  - id: modal-shell
    content: Build CreateTableModal shell with Source/Destination/Schema/Partitioning/Clustering/Tags/Advanced sections
    status: pending
  - id: source-selector
    content: Implement Create-table-from selector with all 8 sources and source-specific fields
    status: pending
  - id: schema-builder
    content: Implement visual schema builder (type/mode, RECORD nesting, REPEATED, RANGE, max length) + Edit as text JSON
    status: pending
  - id: upload-formats
    content: Implement upload + file-format options (CSV/JSONL/Avro/Parquet/ORC) with format-specific controls
    status: pending
  - id: submit
    content: Wire submission to table insert / load job / DDL via explorerQueries; refresh tree + dataset page
    status: pending
  - id: e2e
    content: Add Playwright coverage for creating an empty table with a schema
    status: pending
isProject: false
---

# M3-01: Create Table modal

## Dependencies

- M1 plans; [`m2-01-dataset-detail-page.plan.md`](m2-01-dataset-detail-page.plan.md) (toolbar button entry point).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Datasets → Create Table button.

## Goal

Replace the M2 stub with a full Create table modal matching the BigQuery console, built on the `Modal` primitive.

## Scope

### Source section — "Create table from"

Selector with all options; show source-specific fields:

- **Empty table** (no extra source fields).
- **Google Cloud Storage** — GCS URI field.
- **Upload** — file picker + file format (drives Upload-format options below).
- **Drive** — Drive URI + format.
- **Google Bigtable** — Bigtable URI.
- **Amazon S3** — S3 URI + connection.
- **Azure Blob Storage** — Azure URI + connection.
- **Existing table/view** — source project/dataset/table picker.

Build all options in the UI. For sources the emulator cannot ingest, submit still attempts the real contract and surfaces errors; log the gap in [`upstream-emulator-work.plan.md`](upstream-emulator-work.plan.md).

### Destination section

- Project, Dataset (default to current), Table name with the console's validation helper text.

### Schema section

- Visual builder: per-field name, Type, Mode, Description; Max length for STRING/BYTES; RANGE element type; nested fields for RECORD/STRUCT (recursive add); REPEATED mode.
- **Edit as text** toggle → JSON schema textarea (BQ schema JSON), kept in sync with the builder.
- Validation: required field names, valid nesting, type/mode constraints.

### Partitioning / Clustering / Tags / Advanced

- Partitioning: No partitioning / by ingestion time / by field (+ expiration).
- Clustering: clustering order (fields).
- Tags: scope + key/value pairs.
- Advanced: encryption (Google-managed / Cloud KMS), default collation, default rounding mode.

### Upload-format options (Upload / file sources)

- File format: CSV, JSONL, Avro, Parquet, ORC.
- CSV/text options: write preference, number of errors allowed, unknown values, field delimiter, quote character, source column match, header rows to skip, quoted newlines, jagged rows, null markers, custom timezone & date/time format strings.

### Submit

- Empty/Existing → table insert / `CREATE TABLE`/`CREATE TABLE AS SELECT` as appropriate.
- File/external sources → load job via `explorerQueries.submitJob` + poll `getJob`.
- On success: close modal, refresh dataset Tables list and sidebar tree, optionally open the new table page.
- Surface job errors inline.

## Out of scope

- Copy / Delete / Snapshot / Edit Schema (M3-02).
- Real external-storage emulation (tracked upstream).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: create an empty table with a multi-field nested schema; verify it appears in the tree and dataset Overview.

## Done criteria

- Modal exposes all sources, destination, schema builder + edit-as-text, partitioning/clustering/tags/advanced, and format options.
- Empty-table and existing-table creation succeed against the emulator; other sources attempt the real contract and report errors gracefully.
- New table appears without manual refresh.
