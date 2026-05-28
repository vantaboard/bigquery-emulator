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

package com.example.bigquerydatatransfer;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.truth.Truth.assertWithMessage;

import com.google.cloud.NoCredentials;
import com.google.cloud.bigquery.BigQuery;
import com.google.cloud.bigquery.BigQueryOptions;
import com.google.cloud.bigquery.DatasetInfo;
import com.google.cloud.bigquery.Field;
import com.google.cloud.bigquery.Schema;
import com.google.cloud.bigquery.StandardSQLTypeName;
import com.google.cloud.bigquery.StandardTableDefinition;
import com.google.cloud.bigquery.TableDefinition;
import com.google.cloud.bigquery.TableId;
import com.google.cloud.bigquery.TableInfo;
import com.google.cloud.bigquery.datatransfer.v1.TransferConfig;
import com.google.protobuf.Struct;
import com.google.protobuf.Value;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * bigquery-emulator Phase C live-IT: smoke for {@link CreateAdManagerTransfer}. Models on
 * {@link CreateAmazonS3TransferIT} and routes the BigQuery client through the env-var-aware helper
 * so the setUp dataset/table land in the local emulator instead of live BigQuery. The DTS client
 * itself is constructed by the snippet driver through {@link BqDataTransferOpts}; until Phase D
 * lands a DataTransferService gRPC handler, the assertion below is expected to fail with
 * io.grpc.StatusRuntimeException: UNIMPLEMENTED. See
 * {@code .cursor/plans/java-its-missing-tests_c9d0e1f2.plan.md} for the per-IT verdict baseline.
 */
public class CreateAdManagerTransferIT {

  private static final Logger LOG = Logger.getLogger(CreateAdManagerTransferIT.class.getName());
  private static final String ID = UUID.randomUUID().toString().substring(0, 8);
  private BigQuery bigquery;
  private ByteArrayOutputStream bout;
  private String name;
  private String displayName;
  private String datasetName;
  private String tableName;
  private PrintStream out;
  private PrintStream originalPrintStream;

  private static final String PROJECT_ID = requireEnvVar("GOOGLE_CLOUD_PROJECT");

  private static String requireEnvVar(String varName) {
    String value = System.getenv(varName);
    assertWithMessage("Environment variable %s is required to perform these tests.", varName)
        .that(value)
        .isNotEmpty();
    return value;
  }

  /**
   * Routes the setUp dataset/table at the local emulator when BIGQUERY_EMULATOR_HOST is set.
   * Mirrors the helper baked into CreateAmazonS3TransferIT.
   */
  private static BigQuery newBigQueryService() {
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
    String project = System.getenv("GOOGLE_CLOUD_PROJECT");
    if (project != null && !project.isEmpty()) {
      builder.setProjectId(project);
    }
    return builder.build().getService();
  }

  @BeforeClass
  public static void checkRequirements() {
    requireEnvVar("GOOGLE_CLOUD_PROJECT");
  }

  @Before
  public void setUp() {
    displayName = "MY_AD_MANAGER_TEST_" + ID;
    datasetName = "MY_AD_MANAGER_DS_" + ID;
    tableName = "MY_AD_MANAGER_TBL_" + ID;
    bigquery = newBigQueryService();
    bigquery.create(DatasetInfo.of(datasetName));
    Schema schema =
        Schema.of(
            Field.of("name", StandardSQLTypeName.STRING),
            Field.of("post_abbr", StandardSQLTypeName.STRING));
    TableDefinition tableDefinition = StandardTableDefinition.of(schema);
    TableInfo tableInfo = TableInfo.of(TableId.of(datasetName, tableName), tableDefinition);
    bigquery.create(tableInfo);

    bout = new ByteArrayOutputStream();
    out = new PrintStream(bout);
    originalPrintStream = System.out;
    System.setOut(out);
  }

  @After
  public void tearDown() {
    if (name != null && !name.isEmpty()) {
      try {
        DeleteScheduledQuery.deleteScheduledQuery(name);
      } catch (Exception ignored) {
        // best-effort: when the snippet driver could not create the transfer config (e.g. the
        // emulator's DataTransferService gRPC handler is not wired yet) the delete will also
        // surface UNIMPLEMENTED. The suite must stay re-runnable regardless.
      }
    }
    if (bigquery != null) {
      try {
        bigquery.delete(TableId.of(datasetName, tableName));
      } catch (Exception ignored) {
        // ignore; the table may not have been created on a partial setUp.
      }
      try {
        bigquery.delete(datasetName, BigQuery.DatasetDeleteOption.deleteContents());
      } catch (Exception ignored) {
        // ignore; the dataset may already be gone.
      }
    }
    System.out.flush();
    System.setOut(originalPrintStream);
    LOG.log(Level.INFO, bout.toString());
  }

  @Test
  public void testCreateAdManagerTransfer() throws IOException {
    String bucket = "gs://cloud-sample-data";
    String networkCode = "12345678";
    Map<String, Value> params = new HashMap<>();
    params.put("bucket", Value.newBuilder().setStringValue(bucket).build());
    params.put("network_code", Value.newBuilder().setStringValue(networkCode).build());
    TransferConfig transferConfig =
        TransferConfig.newBuilder()
            .setDestinationDatasetId(datasetName)
            .setDisplayName(displayName)
            .setDataSourceId("dfp_dt")
            .setParams(Struct.newBuilder().putAllFields(params).build())
            .build();
    CreateAdManagerTransfer.createAdManagerTransfer(PROJECT_ID, transferConfig);
    String result = bout.toString();
    if (result.contains("created successfully :")) {
      int idx = result.indexOf("created successfully :") + "created successfully :".length();
      name = result.substring(idx).trim();
    }
    assertThat(result).contains("Ad manager transfer created successfully :");
  }
}
