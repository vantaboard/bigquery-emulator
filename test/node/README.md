# BigQuery Emulator - Node.js/TypeScript Tests

This directory contains Node.js/TypeScript tests for the BigQuery emulator using Vitest and Testcontainers.

## Overview

These tests verify that the BigQuery emulator works correctly with the official `@google-cloud/bigquery` Node.js client library.

## Prerequisites

- Node.js 24+
- Yarn package manager
- Docker (for Testcontainers to launch the emulator)

## Installation

Install dependencies using Yarn:

```bash
cd test/node
yarn install
```

## Running Tests

### Run all tests

```bash
yarn test
```

### Run tests in watch mode

```bash
yarn test:watch
```

### Run tests with coverage

```bash
yarn test:coverage
```

### Type checking only

```bash
yarn typecheck
```

## Environment Variables

- `BIGQUERY_EMULATOR_REPOSITORY` - Docker repository for the emulator image (default: `ghcr.io/vantaboard/bigquery-emulator`)
- `BIGQUERY_EMULATOR_VERSION` - Docker image tag (default: `latest`)
- `VITEST_POOL_ID` - Worker ID for parallel test execution (automatically set by Vitest)


## Contributing

When adding new tests:

1. Follow the existing test structure and naming conventions
2. Use the `BigQueryTestHelper` class for common operations
3. Clean up test data in `afterEach` hooks
4. Add comprehensive assertions to verify behavior
5. Include links to Google documentation and filed issues alongside the testcase

## Related Documentation

- [BigQuery Emulator Main README](../../README.md)
- [Python Testing Guide](../python/README.md)
- [Google Cloud BigQuery Node.js Client](https://github.com/googleapis/nodejs-bigquery)
- [Vitest Documentation](https://vitest.dev/)
- [Testcontainers Node.js](https://node.testcontainers.org/)