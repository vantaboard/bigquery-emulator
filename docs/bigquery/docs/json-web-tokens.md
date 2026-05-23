# Authenticate with JWTs

The BigQuery API accepts
[JSON Web Tokens (JWTs)](https://datatracker.ietf.org/doc/rfc7519/) to
authenticate requests.

As a best practice, you should use
[Application Default Credentials (ADC) to authenticate to BigQuery](https://docs.cloud.google.com/bigquery/docs/authentication).
If you can't use ADC and you're using a service account for authentication, then
you can
[use a signed JWT](https://developers.google.com/identity/protocols/oauth2/service-account#jwt-auth)
instead. JWTs let you make an API call without a network request to Google's
authorization server.

You can use JWTs to authenticate in the following ways:

- For service account keys created in Google Cloud console or by using the gcloud CLI, [use a client library](https://docs.cloud.google.com/bigquery/docs/json-web-tokens#client-libraries) that provides JWT signing.
- For system-managed service accounts, [use the REST API or the gcloud CLI](https://docs.cloud.google.com/bigquery/docs/json-web-tokens#rest-gcloud).

### Scope and Audience

Use [scopes](https://developers.google.com/identity/protocols/oauth2/scopes) with service account when possible. If not possible, you can use an
[audience claim](https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.3).
For the BigQuery APIs, set the audience value to
`https://bigquery.googleapis.com/`.

### Create JWTs with client libraries

For service account keys created in Google Cloud console or by using the
gcloud CLI, use a client library that provides JWT
signing. The following list provides some appropriate options for popular
programming languages:

- Go: [func JWTAccessTokenSourceFromJSON](https://pkg.go.dev/golang.org/x/oauth2/google#JWTAccessTokenSourceFromJSON)
- Java: [Class ServiceAccountCredentials](https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials)
- Node.js: [Class JWTAccess](https://docs.cloud.google.com/nodejs/docs/reference/google-auth-library/latest/google-auth-library/jwtaccess)
- PHP: [ServiceAccountJwtAccessCredentials](https://docs.cloud.google.com/php/docs/reference/cloud-bigquery/latest#authentication)
- Python: [google.auth.jwt module](https://googleapis.dev/python/google-auth/latest/reference/google.auth.jwt.html)
- Ruby: [Class: Google::Auth::ServiceAccountJwtHeaderCredentials](https://www.rubydoc.info/gems/googleauth/Google/Auth/ServiceAccountJwtHeaderCredentials)

#### Java example

The following example uses the
[BigQuery client library for Java](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries)
to create and sign a JWT. The default scope for BigQuery API is set to `https://www.googleapis.com/auth/bigquery` in the client library.

    import com.google.auth.oauth2.https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html;
    import com.google.cloud.bigquery.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html;
    import com.google.common.collect.ImmutableList;

    import java.io.FileInputStream;
    import java.io.IOException;
    import java.net.URI;

    public class Example {
        public static void main(String... args) throws IOException {
            String projectId = "myproject";
            // Load JSON file that contains service account keys and create ServiceAccountCredentials object.
            String credentialsPath = "/path/to/key.json";
            https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.html credentials = null;
            try (FileInputStream is = new FileInputStream(credentialsPath)) {
              credentials =  https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.html.fromStream(is);
              // The default scope for BigQuery is used.
              // Alternatively, use `.setScopes()` to set custom scopes.
              credentials = credentials.https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.html#com_google_auth_oauth2_ServiceAccountCredentials_toBuilder__()
                  .https://docs.cloud.google.com/java/docs/reference/google-auth-library/latest/com.google.auth.oauth2.ServiceAccountCredentials.Builder.html#com_google_auth_oauth2_ServiceAccountCredentials_Builder_setUseJwtAccessWithScope_boolean_(true)
                  .build();
            }
            // Instantiate BigQuery client with the credentials object.
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQuery.html bigquery =
                    https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.BigQueryOptions.html.newBuilder().setCredentials(credentials).build().getService();
            // Use the client to list BigQuery datasets.
            System.out.println("Datasets:");
            bigquery
                .listDatasets(projectId)
                .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.TableResult.html#com_google_cloud_bigquery_TableResult_iterateAll__()
                .forEach(dataset -> System.out.printf("%s%n", dataset.getDatasetId().getDataset()));
        }
    }

### Create JWTs with REST or the gcloud CLI

For system-managed service accounts, you must manually assemble the JWT, then
use the REST method
[`projects.serviceAccounts.signJwt`](https://docs.cloud.google.com/iam/docs/reference/credentials/rest/v1/projects.serviceAccounts/signJwt)
or the Google Cloud CLI command
[`gcloud beta iam service-accounts sign-jwt`](https://cloud.google.com/sdk/gcloud/reference/beta/iam/service-accounts/sign-jwt)
to sign the JWT. To use either of these approaches, you must be a member of the
[Service Account Token Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/iam#iam.serviceAccountTokenCreator)
Identity and Access Management role.

#### gcloud CLI example

The following example shows a bash script that assembles a JWT and then uses the
`gcloud beta iam service-accounts sign-jwt` command to sign it.

    #!/bin/bash

    SA_EMAIL_ADDRESS="myserviceaccount@myproject.iam.gserviceaccount.com"

    TMP_DIR=$(mktemp -d /tmp/sa_signed_jwt.XXXXX)
    trap "rm -rf ${TMP_DIR}" EXIT
    JWT_FILE="${TMP_DIR}/jwt-claim-set.json"
    SIGNED_JWT_FILE="${TMP_DIR}/output.jwt"

    IAT=$(date '+%s')
    EXP=$((IAT+3600))

    cat <<EOF > $JWT_FILE
    {
      "aud": "https://bigquery.googleapis.com/",
      "iat": $IAT,
      "exp": $EXP,
      "iss": "$SA_EMAIL_ADDRESS",
      "sub": "$SA_EMAIL_ADDRESS"
    }
    EOF

    gcloud beta iam service-accounts sign-jwt --iam-account $SA_EMAIL_ADDRESS $JWT_FILE $SIGNED_JWT_FILE

    echo "Datasets:"
    curl -L -H "Authorization: Bearer $(cat $SIGNED_JWT_FILE)" \
    -X GET \
    "https://bigquery.googleapis.com/bigquery/v2/projects/myproject/datasets?alt=json"

## What's next

- Learn more about [BigQuery authentication](https://docs.cloud.google.com/bigquery/docs/authentication).
- Learn how to [authenticate with end-user credentials](https://docs.cloud.google.com/bigquery/docs/authentication/end-user-installed).