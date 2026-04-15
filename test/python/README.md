# BigQuery Emulator - Python Testing Guide

Use the BigQuery emulator in your Python tests with automatic Docker container management via [testcontainers-python](https://testcontainers-python.readthedocs.io/).

## Quick Start

### Installation

```bash
cd test/python
uv sync
```

**Prerequisites**: Python 3.11+, Docker running

### Running Tests

```bash
# Run all tests
uv run pytest

# Run in parallel (automatic port allocation per worker)
uv run pytest -n 4

# Run specific test
uv run pytest emulator_test.py::TestBigQueryEmulator::test_simple_query
```

## Usage

### Option 1: unittest.TestCase

Use `BigQueryEmulatorTestCase` for automatic setup/teardown and helper methods:

```python
from utils.big_query_emulator_test_case import BigQueryEmulatorTestCase, BigQueryAddress
from google.cloud import bigquery

class TestMyQueries(BigQueryEmulatorTestCase):
    def test_simple_query(self):
        result = self.query("SELECT 1 AS col")
        self.assertEqual(result["col"].iloc[0], 1)

    def test_with_table(self):
        address = BigQueryAddress(dataset_id="test_ds", table_id="test_table")

        # Create table
        self.create_mock_table(
            address,
            schema=[
                bigquery.SchemaField("name", "STRING"),
                bigquery.SchemaField("age", "INTEGER"),
            ],
        )

        # Load data
        self.load_rows_into_table(
            address,
            data=[{"name": "Alice", "age": 30}, {"name": "Bob", "age": 25}],
        )

        # Test query
        self.run_query_test(
            f"SELECT name FROM `{self.project_id}.{address.to_str()}` WHERE age > 26",
            expected_result=[{"name": "Alice"}],
        )
```

### Option 2: Pytest Fixtures

Use pytest fixtures for modern test structure:

```python
class TestMyQueries:
    def test_simple_query(self, bq_client):
        result = list(bq_client.query("SELECT 1 AS col"))
        assert result[0]["col"] == 1

    @pytest.mark.usefixtures("clean_emulator_data")
    def test_with_cleanup(self, bq_client):
        bq_client.create_dataset("test_dataset")
        # Data automatically cleaned up after test
```

**Available fixtures:**
- `bq_emulator` - Running emulator container
- `bq_client` - Configured BigQuery client
- `bq_storage_client` - BigQuery Storage API client
- `clean_emulator_data` - Auto cleanup after test

## Key Methods

### BigQueryEmulatorTestCase

```python
# Execute query and get DataFrame
df = self.query("SELECT * FROM dataset.table")

# Create table
self.create_mock_table(address, schema)

# Insert data
self.load_rows_into_table(address, data)

# Test query results
self.run_query_test(query, expected_result, enforce_order=True)

# Check if table exists
if self.table_exists(address):
    ...
```

## Common Patterns

### Complex Types

```python
# Arrays
self.run_query_test(
    "SELECT [1, 2, 3] as numbers",
    expected_result=[{"numbers": [1, 2, 3]}],
)

# Structs
self.run_query_test(
    "SELECT STRUCT('Alice' AS name, 30 AS age) as person",
    expected_result=[{"person": {"name": "Alice", "age": 30}}],
)
```

### JSON Operations

```python
address = BigQueryAddress(dataset_id="test_ds", table_id="json_table")
self.create_mock_table(
    address,
    schema=[bigquery.SchemaField("json_col", "JSON")],
)
self.load_rows_into_table(
    address,
    data=[{"json_col": {"name": "Alice", "age": 30}}],
)

self.run_query_test(
    f"SELECT JSON_VALUE(json_col, '$.name') as name FROM `{self.project_id}.{address.to_str()}`",
    expected_result=[{"name": "Alice"}],
)
```

### BigQuery Storage API

```python
def test_storage_api(self, bq_client, bq_storage_client):
    # Create and populate table
    dataset_ref = bigquery.DatasetReference(project=self.project_id, dataset_id="test_ds")
    bq_client.create_dataset(dataset_ref.dataset_id, exists_ok=True)

    table_ref = bigquery.TableReference(dataset_ref, "test_table")
    table = bq_client.create_table(
        bigquery.Table(table_ref, schema=[bigquery.SchemaField("value", "INTEGER")])
    )
    bq_client.insert_rows_json(table, [{"value": 42}])

    # Read using Storage API
    from google.cloud import bigquery_storage

    session = bigquery_storage.ReadSession(
        table=f"projects/{self.project_id}/datasets/test_ds/tables/test_table",
        data_format=bigquery_storage.DataFormat.ARROW,
    )

    session = bq_storage_client.create_read_session(
        parent=f"projects/{self.project_id}",
        read_session=session,
        max_stream_count=1,
    )

    reader = bq_storage_client.read_rows(session.streams[0].name)
    rows = reader.rows(session).to_arrow()

    assert rows.to_pydict()["value"][0] == 42
```

## Configuration

### Class Attributes

```python
class TestMyQueries(BigQueryEmulatorTestCase):
    # Clean up data after each test (default: True)
    wipe_emulator_data_on_teardown = True

    # Print emulator logs on test failure (default: False)
    show_emulator_logs_on_failure = True
```

### Environment Variables

- `BIGQUERY_EMULATOR_REPOSITORY`: Docker image repo (default: `ghcr.io/vantaboard/bigquery-emulator`)
- `BIGQUERY_EMULATOR_VERSION`: Image version (default: `latest`)

## Troubleshooting

**Container won't start**
```bash
docker ps  # Check Docker is running
```

**Port conflicts**
The emulator uses ports 60050 (HTTP) and 61050 (gRPC). Parallel tests auto-increment ports.

**Import errors**
```bash
uv sync  # Install dependencies
```

**Tests hang in parallel**
```bash
uv run pytest -n 2  # Reduce workers
```

## Examples

See `emulator_test.py` for comprehensive examples including:
- Basic queries and table operations
- Complex types (arrays, structs, JSON)
- Window functions and aggregations
- BigQuery Storage API
- Date/time functions

## License

MIT