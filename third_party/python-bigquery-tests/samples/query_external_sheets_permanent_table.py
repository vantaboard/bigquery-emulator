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


def query_external_sheets_permanent_table(dataset_id: str) -> None:
    # [START bigquery_query_external_sheets_perm]
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
            "https://www.googleapis.com/auth/bigquery",
        ]
    )

    # Construct a BigQuery client object.
    client = bigquery.Client(credentials=credentials, project=project)

    # TODO(developer): Set dataset_id to the ID of the dataset to fetch.
    # dataset_id = "your-project.your_dataset"

    # Configure the external data source (public Google sample sheet, Class Data tab).
    dataset = client.get_dataset(dataset_id)
    table_id = "class_data"
    schema = [
        bigquery.SchemaField("Student Name", "STRING"),
        bigquery.SchemaField("Gender", "STRING"),
        bigquery.SchemaField("Class Level", "STRING"),
        bigquery.SchemaField("Home State", "STRING"),
        bigquery.SchemaField("Major", "STRING"),
        bigquery.SchemaField("Extracurricular Activity", "STRING"),
    ]
    table = bigquery.Table(dataset.table(table_id), schema=schema)
    external_config = bigquery.ExternalConfig("GOOGLE_SHEETS")
    # Public Example Spreadsheet (Class Data tab); matches the emulator fixture
    # snapshot in gateway/external/fixtures/google_sheets/class_data.csv.
    sheet_url = (
        "https://docs.google.com/spreadsheets"
        "/d/1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms/edit"
    )
    external_config.source_uris = [sheet_url]
    options = external_config.google_sheets_options
    assert options is not None
    options.skip_leading_rows = 1  # Optionally skip header row.
    options.range = "Class Data!A1:F31"
    table.external_data_configuration = external_config

    # Create a permanent table linked to the Sheets file.
    table = client.create_table(table)  # Make an API request.

    # Example query: students from home states starting with "W" (WI in fixture).
    sql = (
        "SELECT * FROM `{}.{}` WHERE `Home State` LIKE 'W%'".format(
            dataset_id, table_id
        )
    )

    results = client.query_and_wait(sql)  # Make an API request.

    # Wait for the query to complete.
    w_state_students = list(results)
    print(
        "There are {} students from home states starting with W in the Class Data sheet.".format(
            len(w_state_students)
        )
    )
    # [END bigquery_query_external_sheets_perm]
