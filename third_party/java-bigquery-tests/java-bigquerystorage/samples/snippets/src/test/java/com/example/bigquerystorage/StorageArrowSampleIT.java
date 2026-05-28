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

package com.example.bigquerystorage;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.truth.Truth.assertWithMessage;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * bigquery-emulator missing-tests-follow-up live-IT: smoke for {@link StorageArrowSample}
 * (`bigquerystorage-arrow-quickstart`). Models on {@link QuickstartArrowSampleIT}.
 *
 * <p>The driver routes through {@link BqStorageOpts#newReadClient()} so the Storage Read client
 * dials the local emulator's storage gRPC listener when {@code BIGQUERY_STORAGE_GRPC_ENDPOINT}
 * (or {@code BIGQUERY_EMULATOR_HOST}) is set. Until the gRPC-server follow-up lands a
 * {@code BigQueryRead} gRPC handler (the shallow-emulator {@code bqstorage} skeleton has no
 * Read-path implementation),
 * {@link StorageArrowSample#main(String...)} will surface an
 * {@code io.grpc.StatusRuntimeException: UNIMPLEMENTED} from {@code CreateReadSession} and
 * this test is expected to fail. The IT is committed today so Failsafe discovers it the
 * moment the shallow Read-path port lands; see
 * {@code .cursor/plans/java-its-missing-tests_c9d0e1f2.plan.md}.
 */
@RunWith(JUnit4.class)
@SuppressWarnings("checkstyle:abbreviationaswordinname")
public class StorageArrowSampleIT {
  private static final String PROJECT_ID = System.getenv("GOOGLE_CLOUD_PROJECT");

  private ByteArrayOutputStream bout;
  private PrintStream originalPrintStream;

  @BeforeClass
  public static void checkRequirements() {
    assertWithMessage("Environment variable GOOGLE_CLOUD_PROJECT is required to perform this test.")
        .that(PROJECT_ID)
        .isNotEmpty();
  }

  @Before
  public void setUp() {
    bout = new ByteArrayOutputStream();
    originalPrintStream = System.out;
    System.setOut(new PrintStream(bout));
  }

  @After
  public void tearDown() {
    // restore the original stdout regardless of test outcome so the surrounding suite stays
    // re-runnable. No emulator-side fixture cleanup is required: StorageArrowSample reads from
    // bigquery-public-data and does not create datasets / tables of its own.
    System.out.flush();
    System.setOut(originalPrintStream);
  }

  @Test
  public void testStorageArrowSample() throws Exception {
    StorageArrowSample.main(PROJECT_ID);
    String got = bout.toString();
    assertThat(bout.size()).isGreaterThan(1024);
    assertThat(got).contains("Zayvion");
  }
}
