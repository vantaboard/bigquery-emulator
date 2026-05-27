/*
 * Copyright 2026 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.bigquery;

import com.google.cloud.NoCredentials;
import com.google.cloud.bigquery.BigQueryOptions;

/**
 * Builds a {@link BigQueryOptions} configured for the local bigquery-emulator gateway when the
 * {@code BIGQUERY_EMULATOR_HOST} environment variable is set, falling back to the default
 * application-default-credential path for live BigQuery otherwise.
 *
 * <p>This helper is the bigquery-emulator slim-path equivalent of the Go {@code bqopts} package
 * from go-googlesql. The published {@code google-cloud-bigquery} client does not auto-read
 * {@code BIGQUERY_EMULATOR_HOST} (unlike the Go client), so emulator-aware sample drivers must
 * route through this helper instead of {@link BigQueryOptions#getDefaultInstance()}. See
 * {@code third_party/README.md} (Java section) and {@code java-bigquery/samples/EMULATOR.md}.
 *
 * <p>Recognised env vars:
 *
 * <ul>
 *   <li>{@code BIGQUERY_EMULATOR_HOST} — host:port or http(s)://host:port. Schemeless values are
 *       prefixed with {@code http://}. When set, credentials are forced to {@link NoCredentials}.
 *   <li>{@code GOOGLE_CLOUD_PROJECT} / {@code GCLOUD_PROJECT} / {@code GOLANG_SAMPLES_PROJECT_ID}
 *       — first non-empty wins. Sets {@code BigQueryOptions.projectId} when present.
 * </ul>
 */
public final class BqOpts {

  private BqOpts() {}

  /**
   * Returns a {@link BigQueryOptions.Builder} pre-configured for either the emulator (when
   * {@code BIGQUERY_EMULATOR_HOST} is set) or live BigQuery (default).
   */
  public static BigQueryOptions.Builder builder() {
    BigQueryOptions.Builder builder = BigQueryOptions.newBuilder();

    String host = System.getenv("BIGQUERY_EMULATOR_HOST");
    if (host != null && !host.isEmpty()) {
      String normalized = host;
      if (!normalized.startsWith("http://") && !normalized.startsWith("https://")) {
        if (normalized.startsWith("//")) {
          normalized = normalized.substring(2);
        }
        normalized = "http://" + normalized;
      }
      builder.setHost(normalized).setCredentials(NoCredentials.getInstance());
    }

    String project = firstNonEmpty(
        System.getenv("GOOGLE_CLOUD_PROJECT"),
        System.getenv("GCLOUD_PROJECT"),
        System.getenv("GOLANG_SAMPLES_PROJECT_ID"));
    if (project != null) {
      builder.setProjectId(project);
    }

    return builder;
  }

  private static String firstNonEmpty(String... values) {
    if (values == null) {
      return null;
    }
    for (String value : values) {
      if (value != null && !value.isEmpty()) {
        return value;
      }
    }
    return null;
  }
}
