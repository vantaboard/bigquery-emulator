# Using Sensitive Data Protection to scan BigQuery data

Knowing where your sensitive data exists is often the first step in ensuring
that it is properly secured and managed. This knowledge can help reduce the risk
of exposing sensitive details such as credit card numbers, medical information,
Social Security numbers, driver's license numbers, addresses, full names, and
company-specific secrets. Periodic scanning of your data can also help with
compliance requirements and ensure best practices are followed as your data
grows and changes with use. To help meet compliance requirements, use
Sensitive Data Protection to inspect your BigQuery tables and
to help protect your sensitive data.

There are two ways to scan your BigQuery data:

- **Sensitive data profiling.** Sensitive Data Protection can generate profiles about
  BigQuery data across an organization, folder, or project. *Data
  profiles* contain metrics and metadata about your tables and help you
  determine where [sensitive and high-risk
  data](https://docs.cloud.google.com/sensitive-data-protection/docs/sensitivity-risk-calculation) reside. Sensitive Data Protection
  reports these metrics at the project, table, and column levels. For more
  information, see [Data profiles for
  BigQuery data](https://docs.cloud.google.com/sensitive-data-protection/docs/data-profiles).

- **On-demand inspection.** Sensitive Data Protection can perform a deep inspection on
  a single table or a subset of columns and report its findings down to the cell
  level. This kind of inspection can help you identify individual instances of
  specific data [types](https://docs.cloud.google.com/sensitive-data-protection/docs/infotypes-reference), such as the precise
  location of a credit card number inside a table cell. You can do an on-demand
  inspection through the Sensitive Data Protection page in the
  Google Cloud console, the **BigQuery** page in the Google Cloud console,
  or programmatically through the DLP API.

This page describes how to do an on-demand inspection through the
**BigQuery** page in the Google Cloud console.

Sensitive Data Protection is a fully managed service that lets Google Cloud customers
identify and protect sensitive data at scale. Sensitive Data Protection uses more
than 150 predefined detectors to identify patterns, formats, and checksums.
Sensitive Data Protection also provides a set of tools to de-identify your data
including masking, tokenization, pseudonymization, date shifting, and more, all
without replicating customer data.

To learn more about Sensitive Data Protection, see the [Sensitive Data Protection](https://docs.cloud.google.com/sensitive-data-protection/docs)
documentation.

## Before you begin

1. Get familiar with [Sensitive Data Protection pricing](https://cloud.google.com/sensitive-data-protection/pricing) and [how to keep Sensitive Data Protection costs under control](https://docs.cloud.google.com/sensitive-data-protection/docs/best-practices-costs).
2. [Enable the DLP API](https://docs.cloud.google.com/apis/docs/enable-disable-apis).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=dlp.googleapis.com)
3. Ensure that the user creating your Sensitive Data Protection jobs is granted an
   appropriate predefined Sensitive Data Protection [IAM role](https://docs.cloud.google.com/sensitive-data-protection/docs/iam-roles) or
   sufficient [permissions](https://docs.cloud.google.com/sensitive-data-protection/docs/iam-permissions) to run Sensitive Data Protection
   jobs.

> [!NOTE]
> **Note:** When you enable the DLP API, a service account is created with a name similar to `service-project_number@dlp-api.iam.gserviceaccount.com`. This service account is granted the DLP API Service Agent role, which lets the service account authenticate with the BigQuery API. For more information, see [Service account](https://docs.cloud.google.com/sensitive-data-protection/docs/iam-permissions#service_account) on the Sensitive Data Protection IAM permissions page.

## Scanning BigQuery data using the Google Cloud console

To scan BigQuery data, you create a Sensitive Data Protection job
that analyzes a table. You can scan a BigQuery table quickly by using
the **Scan with Sensitive Data Protection** option in the BigQuery Google Cloud console.

To scan a BigQuery table using Sensitive Data Protection:

1. In the Google Cloud console, go to the BigQuery page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then click your dataset.

4. Click **Overview \> Tables**, and then select your table.

5. Click **Open \> Scan with Sensitive Data Protection**.
   The Sensitive Data Protection job creation page opens in a new tab.

6. For **Step 1: Choose input data** , enter a job ID. The values in the
   **Location** section are automatically generated. Also, the **Sampling**
   section is automatically configured to run a sample scan against your data, but
   you can adjust the settings as needed.

7. Click **Continue**.

8. Optional: For **Step 2: Configure detection** , you can configure what types
   of data to look for, called `infoTypes`.

   Do one of the following:
   - To select from the list of predefined `infoTypes`, click **Manage
     infoTypes**. Then, select the infoTypes you want to search for.
   - To use an existing [inspection template](https://docs.cloud.google.com/sensitive-data-protection/docs/creating-templates-inspect), in the **Template name** field, enter the template's full resource name.

   For more information on `infoTypes`, see
   [InfoTypes and infoType detectors](https://docs.cloud.google.com/sensitive-data-protection/docs/concepts-infotypes) in the
   Sensitive Data Protection documentation.
9. Click **Continue**.

10. Optional: For **Step 3: Add actions** , turn on **Save to BigQuery**
    to publish your Sensitive Data Protection findings to a BigQuery
    table. If you don't store findings, the completed job contains only
    statistics about the number of findings and their `infoTypes`. Saving
    findings to BigQuery saves details about the precise location and
    confidence of each individual finding.

11. Optional: If you turned on **Save to BigQuery** , in the **Save to
    BigQuery** section, enter the following information:

    - **Project ID**: the project ID where your results are stored.
    - **Dataset ID**: the name of the dataset that stores your results.
    - Optional: **Table ID** : the name of the table that stores your results. If no table ID is specified, a default name is assigned to a new table similar to the following: `dlp_googleapis_date_1234567890`. If you specify an existing table, findings are appended to it.

    To include the actual content that was detected, turn on **Include quote**.
12. Click **Continue**.

13. Optional: For **Step 4: Schedule** , configure a time span or schedule by
    selecting either **Specify time span** or **Create a trigger to run the job
    on a periodic schedule**.

14. Click **Continue**.

15. Optional: On the **Review** page, examine the details of your job. If needed,
    adjust the previous settings.

16. Click **Create**.

17. After the Sensitive Data Protection job completes, you are redirected to the job
    details page, and you're notified by email. You can view the results of the
    scan on the job details page, or you can click the link to
    the Sensitive Data Protection job details page in the job completion email.

18. If you chose to publish Sensitive Data Protection findings to
    BigQuery, on the **Job details** page, click **View Findings in
    BigQuery** to open the table in the Google Cloud console. You can then query the
    table and analyze your findings. For more information on querying your results
    in BigQuery, see
    [Querying Sensitive Data Protection findings in BigQuery](https://docs.cloud.google.com/sensitive-data-protection/docs/querying-findings)
    in the Sensitive Data Protection documentation.

## What's next

- Learn more about [inspecting BigQuery and other storage
  repositories for sensitive data using Sensitive Data Protection](https://docs.cloud.google.com/sensitive-data-protection/docs/inspecting-storage).

- Learn more about [profiling data in an organization, folder, or
  project](https://docs.cloud.google.com/sensitive-data-protection/docs/data-profiles).

- Read the Identity \& Security blog post [Take charge of your data: using
  Sensitive Data Protection to de-identify and obfuscate sensitive
  information](https://cloud.google.com/blog/products/identity-security/taking-charge-of-your-data-using-cloud-dlp-to-de-identify-and-obfuscate-sensitive-information).

If you want to redact or otherwise de-identify the sensitive data that the
Sensitive Data Protection scan found, see the following:

- [Inspect text to de-identify sensitive information](https://docs.cloud.google.com/sensitive-data-protection/docs/inspect-sensitive-text-de-identify)
- [De-identifying sensitive data](https://docs.cloud.google.com/sensitive-data-protection/docs/deidentify-sensitive-data) in the Sensitive Data Protection documentation
- [AEAD encryption concepts in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts) for information on encrypting individual values within a table
- [Protecting data with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption) for information on creating and managing your own encryption keys in [Cloud KMS](https://docs.cloud.google.com/kms/docs) to encrypt BigQuery tables