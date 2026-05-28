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
 * bigquery-emulator missing-tests-follow-up live-IT: smoke for {@link CreateRedshiftTransfer}. Same shape as
 * {@link CreateAdManagerTransferIT}; see that class for the DTS-gRPC-UNIMPLEMENTED caveat. AWS
 * credentials below are inline placeholders (never used because the emulator does not perform any
 * Redshift traffic — the CreateTransferConfig is the only RPC under test).
 */
public class CreateRedshiftTransferIT {

  private static final Logger LOG = Logger.getLogger(CreateRedshiftTransferIT.class.getName());
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
    displayName = "MY_REDSHIFT_TEST_" + ID;
    datasetName = "MY_REDSHIFT_DS_" + ID;
    tableName = "MY_REDSHIFT_TBL_" + ID;
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
        // see CreateAdManagerTransferIT.tearDown.
      }
    }
    if (bigquery != null) {
      try {
        bigquery.delete(TableId.of(datasetName, tableName));
      } catch (Exception ignored) {
        // ignore.
      }
      try {
        bigquery.delete(datasetName, BigQuery.DatasetDeleteOption.deleteContents());
      } catch (Exception ignored) {
        // ignore.
      }
    }
    System.out.flush();
    System.setOut(originalPrintStream);
    LOG.log(Level.INFO, bout.toString());
  }

  @Test
  public void testCreateRedshiftTransfer() throws IOException {
    String jdbcUrl = "jdbc:redshift://emulator-fake.invalid:5439/db";
    String dbUserName = "emulator-fake-user";
    String dbPassword = "emulator-fake-password";
    String accessKeyId = "EMULATOR_FAKE_ACCESS_KEY";
    String secretAccessId = "EMULATOR_FAKE_SECRET";
    String s3Bucket = "s3://emulator-fake-bucket";
    String redShiftSchema = "public";
    String tableNamePatterns = "*";
    String vpcAndReserveIpRange = "10.0.0.0/24";
    Map<String, Value> params = new HashMap<>();
    params.put("jdbc_url", Value.newBuilder().setStringValue(jdbcUrl).build());
    params.put("database_username", Value.newBuilder().setStringValue(dbUserName).build());
    params.put("database_password", Value.newBuilder().setStringValue(dbPassword).build());
    params.put("access_key_id", Value.newBuilder().setStringValue(accessKeyId).build());
    params.put("secret_access_key", Value.newBuilder().setStringValue(secretAccessId).build());
    params.put("s3_bucket", Value.newBuilder().setStringValue(s3Bucket).build());
    params.put("redshift_schema", Value.newBuilder().setStringValue(redShiftSchema).build());
    params.put("table_name_patterns", Value.newBuilder().setStringValue(tableNamePatterns).build());
    params.put(
        "migration_infra_cidr", Value.newBuilder().setStringValue(vpcAndReserveIpRange).build());
    TransferConfig transferConfig =
        TransferConfig.newBuilder()
            .setDestinationDatasetId(datasetName)
            .setDatasetRegion("US")
            .setDisplayName(displayName)
            .setDataSourceId("redshift")
            .setParams(Struct.newBuilder().putAllFields(params).build())
            .setSchedule("every 24 hours")
            .build();
    CreateRedshiftTransfer.createRedshiftTransfer(PROJECT_ID, transferConfig);
    String result = bout.toString();
    if (result.contains("created successfully :")) {
      int idx = result.indexOf("created successfully :") + "created successfully :".length();
      name = result.substring(idx).trim();
    }
    assertThat(result).contains("Cloud redshift transfer created successfully :");
  }
}
