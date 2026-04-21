# BigQuery Emulator UI

A web UI for Google BigQuery Emulator, providing a simple interface to interact with local BigQuery emulator instances.

## UI Preview

![UI Screenshot](./docs/ui-screenshot.png)

## Purpose

This project serves as a user-friendly web interface for the BigQuery emulator, allowing developers to:
- Browse emulated BigQuery resources
- Execute and test queries in a local development environment
- Validate schemas and data without connecting to production BigQuery
- Accelerate development and testing workflows

## Features

- Connect to BigQuery emulator or real BigQuery service
- Browse projects, datasets, and tables (multiple projects when the emulator exposes them; see below)
- View table schemas
- Execute queries and get results
- Simple, responsive web interface
- Optional: add projects at runtime against a compatible emulator (`ALLOW_EMULATOR_PROJECT_ADMIN=true`)

### Upstream goccy vs Vantaboard fork

The default `docker-compose.yaml` uses **`ghcr.io/goccy/bigquery-emulator`**, which may not implement the same multi-project discovery or emulator-only HTTP routes as the **[Vantaboard fork](https://github.com/vantaboard/bigquery-emulator)** (`ghcr.io/vantaboard/bigquery-emulator`).

This UI discovers projects by calling **`GET /emulator/v1/projects`** (JSON array of project ids) when available, and falls back to **`GET /bigquery/v2/projects`**.

For local development against the Vantaboard image, use **`docker-compose.local.yaml`** (see [Docker Execution](#docker-execution)).

## Installation

### Prerequisites

- Go 1.23+
- Docker and Docker Compose (for containerized setup)

### Local Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/filipecaixeta/bigquery-emulator-ui.git
   cd bigquery-emulator-ui
   ```

2. Install dependencies:
   ```bash
   go mod download
   ```

3. Create a `.env` file to override defaults (optional):
   ```
   BIGQUERY_EMULATOR_HOST=localhost:9050
   BIGQUERY_PROJECT_IDS=extra-proj,another-proj
   BIGQUERY_PROJECT_IDS_MODE=
   ALLOW_EMULATOR_PROJECT_ADMIN=false
   PORT=8000
   ```

   - **`BIGQUERY_PROJECT_IDS`**: Optional comma-separated list. With **`BIGQUERY_PROJECT_IDS_MODE=override`**, this list replaces discovered projects (if empty, the configured default project is used). Otherwise, these ids are merged with the list returned by the emulator.
   - **`ALLOW_EMULATOR_PROJECT_ADMIN`**: Set to **`true`** to enable **`POST /api/emulator/projects`** and the sidebar “Add emulator project” control (proxies to the emulator’s **`POST /emulator/v1/projects`**). Only use on trusted local emulators.

4. Build the application:
   ```bash
   go build -o bigquery-emulator-ui .
   ```

## Running the Application

### Quick Start

You can run the application directly using:

```bash
BIGQUERY_EMULATOR_HOST=localhost:9050 go run github.com/filipecaixeta/bigquery-emulator-ui@latest
```

### Local Execution

Run the server with command-line flags:

```bash
./bigquery-emulator-ui --project=your-project-id --emulator=localhost:9050
```

Or use environment variables:

```bash
BIGQUERY_EMULATOR_HOST=localhost:9050 ./bigquery-emulator-ui
```

By default, the UI server runs on port 8000. Access it at http://localhost:8000

### Docker Execution

#### Vantaboard emulator (recommended for multi-project)

From this repository directory, run the base compose file plus the local override so the **`bigquery`** service uses the Vantaboard image:

```bash
docker compose -f docker-compose.yaml -f docker-compose.local.yaml up
```

Ports **9050** (HTTP) and **9060** are published as in the base file. The UI service **`bq-ui`** should set **`BIGQUERY_EMULATOR_HOST=bigquery:9050`** (already in `docker-compose.yaml`).

To build the emulator from a **sibling** source tree instead of pulling the image, see the comments in [`docker-compose.local.yaml`](docker-compose.local.yaml) (requires the full stack context described in the **bigquery-emulator** repo, e.g. `Dockerfile.linked`).

#### Default compose (goccy image)

If you already have a BigQuery emulator running, you can use this UI with it:

1. Create a minimal `docker-compose.yaml`:
   ```yaml
    version: '3.7'
    services:
        bigquery:
            platform: linux/x86_64
            image: ghcr.io/goccy/bigquery-emulator:latest
            volumes:
                - ./data:/data
            command: --project=local-project --data-from-yaml=/data/data.yaml
        bq-ui:
            platform: linux/x86_64
            image: ghcr.io/filipecaixeta/bigquery-emulator-ui:latest
            ports:
                - "8000:8000"
            environment:
                - BIGQUERY_EMULATOR_HOST=bigquery:9050
            depends_on:
                - bigquery
   ```

2. Start the UI container:
   ```bash
   docker-compose up -d
   ```

Access the UI at http://localhost:8000

#### Environment Variables for Docker

You can customize the Docker environment by modifying the docker-compose.yaml file or by creating a .env file with the following variables:

- `BIGQUERY_EMULATOR_HOST`: Host and port of the emulator (**`bigquery:9050`** inside Compose; **`localhost:9050`** when the UI runs on your machine and the emulator publishes **9050**).
- `BIGQUERY_PROJECT_IDS` / `BIGQUERY_PROJECT_IDS_MODE`: Optional project list merge or override (see above).
- `ALLOW_EMULATOR_PROJECT_ADMIN`: Set **`true`** only for trusted local emulators to enable add-project UI and API.
- `PORT`: The port for the UI server (default: 8000)

### Running the UI on the host against a local emulator

With the emulator listening on **localhost:9050**:

```bash
BIGQUERY_EMULATOR_HOST=localhost:9050 go run .
```

Open **http://localhost:8000**. The resource tree lists all projects returned by the emulator (when supported).

### Verification (multi-project)

1. Start a Vantaboard-based emulator (e.g. `docker compose -f docker-compose.yaml -f docker-compose.local.yaml up`).
2. Ensure at least two projects exist (seed data with two projects, or add one with  
   `curl -sS -X POST "http://localhost:9050/emulator/v1/projects" -H "Content-Type: application/json" -d '{"id":"second-project"}'`).
3. Open the UI and confirm **`GET /api/projects`** (via the sidebar tree) lists both projects and that datasets load per project when expanded.
4. With **`ALLOW_EMULATOR_PROJECT_ADMIN=true`**, use the sidebar “Add emulator project” or **`POST /api/emulator/projects`** with body `{"id":"..."}` and confirm the list refreshes.

## Development

To run in development mode with hot reloading:

```bash
go run . --project=your-project-id --emulator=localhost:9050
```

## CI/CD Pipeline

This project uses GitHub Actions to automatically build and publish Docker images to GitHub Container Registry.

- Images are built on every push to main/master branch and for pull requests
- When pushing tags (v1.0.0, etc.), versioned images are created
- Latest images are available at `ghcr.io/filipecaixeta/bigquery-emulator-ui:latest`
- Versioned images follow semantic versioning: `ghcr.io/filipecaixeta/bigquery-emulator-ui:v1.0.0`


## Troubleshooting

- **Connection issues**: Ensure the BigQuery emulator is running and accessible
- **Authentication errors**: When using real BigQuery, check your GCP credentials
- **Empty projects list**: Verify the project ID matches what's configured in the emulator

