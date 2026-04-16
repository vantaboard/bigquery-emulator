/**
 * Testcontainers wrapper for the BigQuery Emulator.
 *
 * This module provides a testcontainers-based implementation for managing
 * BigQuery emulator containers in tests, similar to the Python implementation.
 */

import {
  GenericContainer,
  StartedTestContainer,
  Wait,
} from 'testcontainers';

export const BQ_EMULATOR_PROJECT_ID = 'bigquery-emulator-test';

/**
 * Gets the Docker image to use for the BigQuery emulator.
 * Can be overridden via environment variables.
 */
function getEmulatorImage(): string {
  const repository =
    process.env.BIGQUERY_EMULATOR_REPOSITORY ||
    'ghcr.io/vantaboard/bigquery-emulator';
  const version = process.env.BIGQUERY_EMULATOR_VERSION || 'latest';
  return `${repository}:${version}`;
}

/**
 * Generates a unique port for the HTTP endpoint based on worker ID.
 * This allows parallel test execution without port conflicts.
 */
function getBqEmulatorPort(): number {
  const basePort = 60050;
  const workerId = parseInt(process.env.VITEST_POOL_ID || '0', 10);
  return basePort + workerId;
}

/**
 * Generates a unique port for the gRPC endpoint based on worker ID.
 * This allows parallel test execution without port conflicts.
 */
function getBqEmulatorGrpcPort(): number {
  const basePort = 61050;
  const workerId = parseInt(process.env.VITEST_POOL_ID || '0', 10);
  return basePort + workerId;
}

export interface BigQueryEmulatorOptions {
  image?: string;
  port?: number;
  grpcPort?: number;
  inputSchemaJsonPath?: string;
}

/**
 * Testcontainers implementation for BigQuery emulator.
 *
 * This class provides a testcontainers-based wrapper for managing BigQuery
 * emulator containers in tests. It supports:
 * - Automatic port allocation for parallel testing
 * - Volume mounting for loading test data
 * - Built-in wait strategies for container readiness
 * - Automatic cleanup
 *
 * Example:
 *   const emulator = await BigQueryEmulatorContainer.start();
 *   try {
 *     const client = new BigQuery({
 *       projectId: BQ_EMULATOR_PROJECT_ID,
 *       apiEndpoint: emulator.getConnectionUrl(),
 *     });
 *     // Use client for testing
 *   } finally {
 *     await emulator.stop();
 *   }
 */
export class BigQueryEmulatorContainer {
  private container: StartedTestContainer | null = null;
  public readonly port: number;
  public readonly grpcPort: number;
  private readonly image: string;
  private readonly inputSchemaJsonPath?: string;

  constructor(options: BigQueryEmulatorOptions = {}) {
    this.image = options.image || getEmulatorImage();
    this.port = options.port || getBqEmulatorPort();
    this.grpcPort = options.grpcPort || getBqEmulatorGrpcPort();
    this.inputSchemaJsonPath = options.inputSchemaJsonPath;
  }

  /**
   * Starts the BigQuery emulator container.
   */
  async start(): Promise<void> {
    const entrypoint = '/bin/bigquery-emulator';
    let command = [
      `--project=${BQ_EMULATOR_PROJECT_ID}`,
      '--log-level=info',
      '--database=:memory:',
      `--port=${this.port}`,
      `--grpc-port=${this.grpcPort}`,
    ];

    let containerBuilder = new GenericContainer(this.image)
      .withCommand(command)
      .withExposedPorts(
        { container: this.port, host: this.port },
        { container: this.grpcPort, host: this.grpcPort }
      )
      .withWaitStrategy(Wait.forLogMessage('REST server listening at 0.0.0.0'));

    // Handle volume mounting for test data if provided
    if (this.inputSchemaJsonPath) {
      const fileName = this.inputSchemaJsonPath.split('/').pop();
      command.push(`--data-from-json=/fixtures/${fileName}`);
      const directory = this.inputSchemaJsonPath
        .split('/')
        .slice(0, -1)
        .join('/');
      containerBuilder = containerBuilder.withBindMounts([
        {
          source: directory,
          target: '/fixtures',
          mode: 'rw',
        },
      ]);
    }

    this.container = await containerBuilder.start();
  }

  /**
   * Stops the BigQuery emulator container.
   */
  async stop(): Promise<void> {
    if (this.container) {
      await this.container.stop();
      this.container = null;
    }
  }

  /**
   * Get the HTTP connection URL for the emulator.
   *
   * Returns URL string (e.g., "http://0.0.0.0:60050")
   */
  getConnectionUrl(): string {
    return `http://0.0.0.0:${this.port}`;
  }

  /**
   * Get the gRPC endpoint for the emulator.
   *
   * Returns endpoint string (e.g., "localhost:61050")
   */
  getGrpcEndpoint(): string {
    return `localhost:${this.grpcPort}`;
  }

  /**
   * Get container logs (useful for debugging).
   */
  async getLogs(): Promise<string> {
    if (!this.container) {
      return '';
    }
    const logs = await this.container.logs();
    return logs.toString();
  }

  /**
   * Convenience method to start a container and return it.
   * Combines construction and starting in one call.
   */
  static async start(
    options: BigQueryEmulatorOptions = {}
  ): Promise<BigQueryEmulatorContainer> {
    const instance = new BigQueryEmulatorContainer(options);
    await instance.start();
    return instance;
  }
}
