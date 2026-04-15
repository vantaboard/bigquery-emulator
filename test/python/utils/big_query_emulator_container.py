"""Testcontainers wrapper for the BigQuery Emulator.

This module provides a testcontainers-based implementation for managing
BigQuery emulator containers in tests.
"""
import os
from testcontainers.core.container import DockerContainer
from testcontainers.core.wait_strategies import LogMessageWaitStrategy

BQ_EMULATOR_PROJECT_ID = "bigquery-emulator-test"


def get_pytest_worker_id() -> int:
    """Retrieves the worker number from the appropriate environment variable
    https://github.com/pytest-dev/pytest-xdist#identifying-the-worker-process-during-a-test
    """
    return int(os.environ.get("PYTEST_XDIST_WORKER", "gw0")[2:])


def get_bq_emulator_port() -> int:
    """Returns the port for the BigQuery emulator that the current environment should
    use. If we are running outside the context of pytest, we assume there is only one
    emulator running at the default port. Otherwise, the port number is variable to
    avoid interference between tests running in parallel.
    """
    # This port is arbitrarily chosen
    port = 60050
    if not (pytest_worker_id := get_pytest_worker_id()):
        return port

    return port + pytest_worker_id


def get_bq_emulator_grpc_port() -> int:
    """Returns the port for the BigQuery emulator that the current environment should
    use. If we are running outside the context of pytest, we assume there is only one
    emulator running at the default port. Otherwise, the port number is variable to
    avoid interference between tests running in parallel.
    """
    # This port is arbitrarily chosen
    port = 61050
    if not (pytest_worker_id := get_pytest_worker_id()):
        return port

    return port + pytest_worker_id


EMULATOR_IMAGE_REPOSITORY = os.getenv("BIGQUERY_EMULATOR_REPOSITORY", "ghcr.io/vantaboard/bigquery-emulator")
EMULATOR_VERSION = os.getenv("BIGQUERY_EMULATOR_VERSION", "latest")
EMULATOR_IMAGE = f"{EMULATOR_IMAGE_REPOSITORY}:{EMULATOR_VERSION}"

EMULATOR_ENTRYPOINT = "/bin/bigquery-emulator"


class BigQueryEmulatorContainer(DockerContainer):
    """Testcontainers implementation for BigQuery emulator.

    This class provides a testcontainers-based wrapper for managing BigQuery
    emulator containers in tests. It supports:
    - Automatic port allocation for parallel testing with pytest-xdist
    - Volume mounting for loading test data
    - Built-in wait strategies for container readiness
    - Automatic cleanup

    Example:
        >>> with BigQueryEmulatorContainer() as emulator:
        ...     client = bigquery.Client(
        ...         project=BQ_EMULATOR_PROJECT_ID,
        ...         client_options=ClientOptions(
        ...             api_endpoint=emulator.get_connection_url()
        ...         ),
        ...         credentials=credentials.AnonymousCredentials(),
        ...     )
        ...     # Use client for testing
    """

    def __init__(
        self,
        image: str | None = None,
        port: int | None = None,
        grpc_port: int | None = None,
        input_schema_json_path: str | None = None,
        **kwargs
    ):
        """Initialize BigQuery emulator container.

        Args:
            image: Docker image to use. Defaults to value from environment variables
                   BIGQUERY_EMULATOR_REPOSITORY and BIGQUERY_EMULATOR_VERSION.
            port: HTTP port to expose. Defaults to port based on pytest worker ID.
            grpc_port: gRPC port to expose. Defaults to port based on pytest worker ID.
            input_schema_json_path: Optional path to JSON file containing schema data
                                   to load on startup.
            **kwargs: Additional arguments to pass to DockerContainer.
        """
        image = image or EMULATOR_IMAGE
        super().__init__(image, **kwargs)

        self.port = port or get_bq_emulator_port()
        self.grpc_port = grpc_port or get_bq_emulator_grpc_port()
        self.input_schema_json_path = input_schema_json_path

        # Configure ports - bind container ports to host ports
        self.with_bind_ports(self.port, self.port)
        self.with_bind_ports(self.grpc_port, self.grpc_port)

        # Build command
        command = (
            f"{EMULATOR_ENTRYPOINT} --project={BQ_EMULATOR_PROJECT_ID} "
            f"--log-level=info --database=:memory: --port={self.port} "
            f"--grpc-port={self.grpc_port}"
        )

        # Handle volume mounting for test data
        if input_schema_json_path:
            file_name = os.path.basename(input_schema_json_path)
            command += f" --data-from-json=/fixtures/{file_name}"
            self.with_volume_mapping(
                os.path.dirname(input_schema_json_path),
                "/fixtures",
                mode="rw"
            )

        self.with_command(command)

        # Wait for container to be ready
        # The emulator logs "REST server listening at 0.0.0.0" when ready
        self.waiting_for = LogMessageWaitStrategy("REST server listening at 0.0.0.0")

    def get_connection_url(self) -> str:
        """Get the HTTP connection URL for the emulator.

        Returns:
            URL string (e.g., "http://0.0.0.0:60050")
        """
        return f"http://0.0.0.0:{self.port}"

    def get_grpc_endpoint(self) -> str:
        """Get the gRPC endpoint for the emulator.

        Returns:
            Endpoint string (e.g., "localhost:61050")
        """
        return f"localhost:{self.grpc_port}"
