# BigQuery Node samples and the go-googlesql emulator

When **`BIGQUERY_EMULATOR_HOST`** is set, merge **`getBigQueryClientOptions()`** from
`lib/bigqueryEmulatorClientOptions.js` into **`new BigQuery({ ... })`** (see
`setUserAgent.js` and `setClientEndpoint.js`).

## Environment

| Variable | Purpose |
| -------- | ------- |
| **`BIGQUERY_EMULATOR_HOST`** | HTTP BigQuery REST listener. You may use `host:port` or `http://host:port`. A host without a scheme is normalized to **`http://…`** so the client does not default to HTTPS against the local emulator. |
| **`NODE_SAMPLES_PROJECT_ID`** | Optional explicit project id for the client (also reads **`GOLANG_SAMPLES_PROJECT_ID`**, **`GOOGLE_CLOUD_PROJECT`**, **`GCLOUD_PROJECT`**). |
| **`GOLANG_SAMPLES_PROJECT_ID`** | Same as go-googlesql / **`.envrc`**: used when the Google project env vars are unset. |
| **`EMULATOR_PROJECT_ID`** | With **`BIGQUERY_EMULATOR_HOST`** only, defaults the sample project id to this value (else **`dev`**). |
| **`BIGQUERY_EMULATOR_CLIENT_API_REGION`** | Optional region sent as **`X-BigQuery-Emulator-Api-Region`** on each request so the emulator can apply regional rules while the TCP host is loopback (same idea as **golang-samples** `bqopts`). |


Mocha runs **`test/setup.js`** first (`npm test` / **`package.json`** **`--require`**). That copies the resolved project id into **`GOOGLE_CLOUD_PROJECT`** and **`GCLOUD_PROJECT`** when unset so **`new BigQuery()`** does not call **`findAndCacheProjectId`** against metadata. When **`BIGQUERY_EMULATOR_HOST`** is set, **`test/setup.js`** also wraps **`@google-cloud/bigquery`**’s **`BigQuery`** constructor so a regional **`apiEndpoint`** (e.g. **`https://us-east4-bigquery.googleapis.com`**) is mirrored into **`X-BigQuery-Emulator-Api-Region`** on each HTTP request (see **`lib/endpointRegionFromApiEndpoint.js`**), matching **`api/apiregion/policy.go`** without editing individual tests.

The **`@google-cloud/bigquery`** library also reads **`BIGQUERY_EMULATOR_HOST`** directly in its
constructor; the helpers above only add scheme normalization, project id, and the optional
region header.

## Smoke test

From this directory, with the emulator running and **`BIGQUERY_EMULATOR_HOST`** exported (for
example from the parent repo’s **`.envrc`**):

```bash
npm install
npx mocha test/clients.test.js --timeout 200000 --require ./test/setup.js
```

Without **`BIGQUERY_EMULATOR_HOST`**, the same tests assert regional **`*.googleapis.com`**
endpoints as in the upstream samples.

## Full suite

**`npm test`** runs all Mocha files under **`test/`**. Against **production GCP**, that path
expects **Application Default Credentials** and a real project unless individual tests skip.
With **`BIGQUERY_EMULATOR_HOST`** (and optional **`STORAGE_EMULATOR_HOST`**, gRPC endpoints, project
id env vars)—as in this repo’s **`.envrc`** or **GitHub Actions**—requests go to **go-googlesql**’s
BigQuery emulator and **fake-gcs-server** where the samples and client support it.

The parent repo’s task is **`task thirdparty:node-bigquery-tests`** (same as **`npm test`** here).

When **`BIGQUERY_EMULATOR_HOST`** is set, **`test/setup.js`** also:

- Skips entire **`models.test.js`** (BQML / `google_analytics_sample`). All
  other Mocha files run when the gateway loads
  `testdata/public-data/bigquery-public-data.yaml` at startup (including
  `jobs.test.js`, legacy SQL queries, and cross-project public-dataset listing).

**CI / narrow checks:** from this directory you can run **`npx mocha test/clients.test.js --timeout 200000 --require ./test/setup.js`** after **`npm install`** for lightweight client setup checks (see **Smoke test** above).

**CI:** In **go-googlesql** **`.github/workflows/thirdparty-samples.yml`**, the **nodejs-bigquery**
job builds **`cmd/bq-emulator`**, starts **Docker Compose `fake-gcs-server`**, exports the same
**`BIGQUERY_EMULATOR_HOST`** / **`STORAGE_EMULATOR_HOST`** / gRPC / project env vars as local **`.envrc`**,
then runs **`npm test`** under **`third_party/node-bigquery-tests`**.
