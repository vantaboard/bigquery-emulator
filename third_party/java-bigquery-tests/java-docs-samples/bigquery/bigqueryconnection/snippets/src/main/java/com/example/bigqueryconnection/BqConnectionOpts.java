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

package com.example.bigqueryconnection;

import com.google.api.gax.core.NoCredentialsProvider;
import com.google.api.gax.grpc.InstantiatingGrpcChannelProvider;
import com.google.cloud.bigqueryconnection.v1.ConnectionServiceClient;
import com.google.cloud.bigqueryconnection.v1.ConnectionServiceSettings;
import java.io.IOException;

/**
 * Builds {@link ConnectionServiceSettings} configured for the local bigquery-emulator gRPC listener
 * when {@code BIGQUERY_STORAGE_GRPC_ENDPOINT} (or {@code BIGQUERY_EMULATOR_HOST}) is set, falling
 * back to the default application-default-credential path for live BigQuery Connection otherwise.
 *
 * <p>Sibling of {@link com.example.bigquery.BqOpts} for the {@code bigqueryconnection-*} sample
 * IDs. See {@code third_party/README.md} (Java section) and
 * {@code java-bigquery/samples/EMULATOR.md} for the env-var contract.
 *
 * <p>Recognised env vars:
 *
 * <ul>
 *   <li>{@code BIGQUERY_STORAGE_GRPC_ENDPOINT} — host:port for the emulator's gRPC listener (the
 *       local emulator publishes :9060 by default; see {@code docker-compose.yml}). Takes
 *       precedence over {@code BIGQUERY_EMULATOR_HOST}.
 *   <li>{@code BIGQUERY_EMULATOR_HOST} — host:port or {@code http(s)://host:port} of the REST
 *       gateway; used as a fallback (strips the scheme and reuses the host portion when the storage
 *       endpoint is not set explicitly).
 * </ul>
 *
 * <p>When either env var is set the channel provider is forced to plaintext (no TLS) and
 * credentials are forced to {@link NoCredentialsProvider}; this matches the local emulator's
 * listener.
 */
public final class BqConnectionOpts {

  private BqConnectionOpts() {}

  /** Returns the resolved emulator gRPC endpoint, or {@code null} if no env var hint is set. */
  public static String emulatorEndpoint() {
    String storage = System.getenv("BIGQUERY_STORAGE_GRPC_ENDPOINT");
    if (storage != null && !storage.isEmpty()) {
      return stripScheme(storage);
    }
    String host = System.getenv("BIGQUERY_EMULATOR_HOST");
    if (host != null && !host.isEmpty()) {
      return stripScheme(host);
    }
    return null;
  }

  /**
   * Returns a {@link ConnectionServiceSettings.Builder} pre-configured for either the local
   * emulator (plaintext + no credentials) or live BigQuery Connection (default ADC) depending on
   * env-var presence.
   */
  public static ConnectionServiceSettings.Builder builder() throws IOException {
    ConnectionServiceSettings.Builder builder = ConnectionServiceSettings.newBuilder();
    String endpoint = emulatorEndpoint();
    if (endpoint != null) {
      builder
          .setTransportChannelProvider(
              InstantiatingGrpcChannelProvider.newBuilder()
                  .setEndpoint(endpoint)
                  .setChannelConfigurator(channel -> channel.usePlaintext())
                  .build())
          .setCredentialsProvider(NoCredentialsProvider.create());
    }
    return builder;
  }

  /** Convenience: build a {@link ConnectionServiceClient} wired through {@link #builder()}. */
  public static ConnectionServiceClient newClient() throws IOException {
    return ConnectionServiceClient.create(builder().build());
  }

  private static String stripScheme(String value) {
    String stripped = value;
    if (stripped.startsWith("http://")) {
      stripped = stripped.substring("http://".length());
    } else if (stripped.startsWith("https://")) {
      stripped = stripped.substring("https://".length());
    }
    if (stripped.startsWith("//")) {
      stripped = stripped.substring(2);
    }
    return stripped;
  }
}
