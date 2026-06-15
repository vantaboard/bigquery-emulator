# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


def query_external_sheets_temporary_table() -> None:
    # [START bigquery_query_external_sheets_temp]
    # [START bigquery_auth_drive_scope]
    from google.cloud import bigquery
    import google.auth

    # Create credentials with Drive & BigQuery API scopes.
    # Both APIs must be enabled for your project before running this code.
    #
    # If you are using credentials from gcloud, you must authorize the
    # application first with the following command:
    #
    # gcloud auth application-default login \
    #   --scopes=https://www.googleapis.com/auth/drive,https://www.googleapis.com/auth/cloud-platform
    credentials, project = google.auth.default(
        scopes=[
            "https://www.googleapis.com/auth/drive",
            "https://www.googleapis.com/auth/cloud-platform",
        ]
    )

    # Construct a BigQuery client object.
    client = bigquery.Client(credentials=credentials, project=project)
    # [END bigquery_auth_drive_scope]

    # Configure the external data source and query job (Class Data sample sheet).
    external_config = bigquery.ExternalConfig("GOOGLE_SHEETS")

    sheet_url = (
        "https://docs.google.com/spreadsheets"
        "/d/1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms/edit"
    )
    external_config.source_uris = [sheet_url]
    external_config.schema = [
        bigquery.SchemaField("Student Name", "STRING"),
        bigquery.SchemaField("Gender", "STRING"),
        bigquery.SchemaField("Class Level", "STRING"),
        bigquery.SchemaField("Home State", "STRING"),
        bigquery.SchemaField("Major", "STRING"),
        bigquery.SchemaField("Extracurricular Activity", "STRING"),
    ]
    options = external_config.google_sheets_options
    assert options is not None
    options.skip_leading_rows = 1  # Optionally skip header row.
    options.range = "Class Data!A1:F31"
    table_id = "class_data"
    job_config = bigquery.QueryJobConfig(table_definitions={table_id: external_config})

    sql = 'SELECT * FROM `{}` WHERE `Home State` LIKE "W%"'.format(table_id)

    query_job = client.query(sql, job_config=job_config)  # Make an API request.

    # Wait for the query to complete.
    w_state_students = list(query_job)
    print(
        "There are {} students from home states starting with W in the Class Data sheet.".format(
            len(w_state_students)
        )
    )
    # [END bigquery_query_external_sheets_temp]
