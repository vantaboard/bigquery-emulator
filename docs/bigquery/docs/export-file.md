# Export query results to a file

This document describes how to save query results as a file, such as CSV or JSON.

## Download query results to a local file

Downloading query results to a local file is not supported by the bq command-line tool
or the API.

To download query results as a CSV or newline-delimited JSON file, use the
Google Cloud console:

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **SQL query**.

3. Enter a valid GoogleSQL query in the **Query editor** text area.

4. Optional: To change the processing location, click **More** and select
   **Query settings** . For **Data location** ,
   choose the [location](https://docs.cloud.google.com/bigquery/docs/locations) of your data.

5. Click **Run**.

6. When the results are returned, click **Save results** and select the
   format or location where you want to save the results.

   The file is downloaded to your browser's default download location.

## Save query results to Google Drive

> [!WARNING]
>
> **Beta**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

Saving query results to Google Drive is not supported by the bq command-line tool or
the API.

You might get an error when you try to save the BigQuery results
to Google Drive. This error is due to the Drive SDK API
being unable to access Google Workspace. To resolve the issue,
you must enable your user account to
[access Google Drive](https://support.google.com/a/answer/6105699)
with the Drive SDK API.

To save query results to Google Drive, use the Google Cloud console:

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **SQL query**.

3. Enter a valid GoogleSQL query in the **Query editor** text area.

4. Click **Run**.

5. When the results are returned, click **Save results**.

6. Under **Google Drive** , select **CSV** or **JSON**. When you save
   results to Google Drive, you cannot choose the location. Results are
   always saved to the root "My Drive" location.

7. It may take a few minutes to save the results to Google Drive. When
   the results are saved, you receive a dialog message that includes the
   filename ---
   `bq-results-[TIMESTAMP]-[RANDOM_CHARACTERS].[CSV or JSON]`.


   ![screenshot of save results button](https://docs.cloud.google.com/static/bigquery/images/save_results_notification_open.png)
8. In the dialog message, click **Open** to open the file, or navigate to
   Google Drive and click **My Drive**.

## Save query results to Google Sheets

Saving query results to Google Sheets is not supported by the bq command-line tool or
the API.

You might get an error when you try to open the BigQuery results
from Google Sheets. This error is due to the Drive SDK API
being unable to access Google Workspace. To resolve the issue,
you must enable your user account to
[access Google Sheets](https://support.google.com/a/answer/6105699)
with the Drive SDK API.

To save query results to Google Sheets, use the Google Cloud console:

1. In the Google Cloud console, open the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **SQL query**.

3. Enter a valid GoogleSQL query in the **Query editor** text area.

4. Optional: To change the processing location, click **More** and select
   **Query settings** . For **Data location** ,
   choose the [location](https://docs.cloud.google.com/bigquery/docs/locations) of your data.

5. Click **Run**.

6. When the results are returned, click the **Save results** and select
   **Google Sheets**.

7. If necessary, follow the prompts to log into your user account and
   click **Allow** to give BigQuery permission to write the data
   to your Google Drive `My Drive` folder.

   After following the prompts, you should receive an email confirming that
   BigQuery client tools have been connected to your user
   account. The email contains information on the permissions you granted
   along with steps to remove the permissions.
8. When the results are saved, a message similar to the following appears
   below the query results in the Google Cloud console: `Saved to Sheets as
   "results-20190225-103531"`. Click the link in the message to view your
   results in Google Sheets, or navigate to your `My Drive` folder and open the
   file manually.

   When you save query results to Google Sheets, the filename begins with
   `results-[DATE]` where `[DATE]` is today's date in the format
   `YYYYMMDD`.

   > [!NOTE]
   > **Note:** Saving results to Google Sheets is not supported by the bq command-line tool or the API. For more information, see [Using Connected Sheets](https://docs.cloud.google.com/bigquery/docs/connected-sheets).

### Troubleshoot saving results to Google Sheets

When saving data from BigQuery to Google Sheets, you might
find that some cells in the sheets are blank. This happens when the data you
are writing to the cell exceeds the Google Sheets limit of 50,000 characters.
To resolve this, use a
[string function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/string_functions#split)
in the GoogleSQL query to split the column with the long data into two or more
columns, then save the result to sheets again.

## Save query results to Cloud Storage

You can export your query results to Cloud Storage in the Google Cloud console
with the following steps:

1. Open the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. Click **SQL query**.

3. Enter a valid GoogleSQL query in the **Query editor** text area.

4. Click **Run**.

5. When the results are returned, click **Save results** \> **Cloud Storage**.

6. In the **Export to Google Cloud Storage** dialog:

   - For **GCS Location**, browse for the bucket, folder, or file where you want to export the data.
   - For **Export format**, choose the format for your exported data: CSV, JSON (Newline Delimited), Avro, or Parquet.
   - For **Compression** , select a compression format or select `None` for no compression.
7. Click **Save** to export the query results.

To check on the progress of the job, expand the **Job history** pane and
look for the job with the `EXTRACT` type.

## Restrict downloading query results

To prevent users from downloading query results from the Google Cloud console, use
one of the following methods:

- Configure a [VPC Service Controls](https://docs.cloud.google.com/vpc-service-controls/docs/overview)
  perimeter to prevent data exfiltration. This blocks users from downloading
  and exporting data outside the established perimeter boundaries.

- Contact [Cloud Customer Care](https://docs.cloud.google.com/support) to request
  that your Google Cloud project or organization be added to a
  restricted list. This disables the data download and export options directly
  within the Google Cloud console.

## What's next

- Learn how to programmatically [export a table to a JSON file](https://docs.cloud.google.com/bigquery/docs/samples/bigquery-extract-table-json).
- Learn about [quotas for extract jobs](https://docs.cloud.google.com/bigquery/quotas#export_jobs).
- Learn about [BigQuery storage pricing](https://cloud.google.com/bigquery/pricing#storage).