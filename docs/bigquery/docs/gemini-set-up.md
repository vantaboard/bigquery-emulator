# Set up Gemini in BigQuery

Before you can use
[Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview), which
offers AI-powered assistance for your data analytics, your team must enable
required APIs and grant roles.

To learn more about using Gemini in BigQuery features,
see [Gemini in BigQuery
pricing](https://cloud.google.com/products/gemini/pricing#gemini-in-bigquery-pricing).

> [!NOTE]
> **Note** : Gemini in BigQuery is part of Gemini for Google Cloud and doesn't support the same compliance and security offerings as BigQuery. You should only set up Gemini in BigQuery for BigQuery projects that don't require [compliance offerings that aren't supported by Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications). For information about how to turn off or prevent access to Gemini in BigQuery, see [Turn off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off).

## Enable required APIs and grant roles

To use Gemini in BigQuery,
you must enable required APIs and grant required Identity and Access Management (IAM)
roles. A service administrator or project
owner with the [`serviceusage.services.enable` IAM permission](https://docs.cloud.google.com/service-usage/docs/access-control#predefined_roles)
usually performs this step. For a list of APIs and services used by
BigQuery, see [Manage BigQuery API dependencies](https://docs.cloud.google.com/bigquery/docs/service-dependencies).

1. In the the Google Cloud console, with your project selected, go to the
   **BigQuery Studio** page.

   [Go to BigQuery Studio](https://console.cloud.google.com/bigquery)
2. View a Gemini in BigQuery feature in the
   the Google Cloud console. For example, in BigQuery Studio hover
   over the pen_spark
   **Gemini** icon.

   ![Gemini button in the BigQuery toolbar.](https://docs.cloud.google.com/static/gemini/images/gemini-assistant-link-disabled.png)

   The console prompts you to enable additional Google Cloud APIs.
3. Click **Continue** to start enabling required
   Google Cloud APIs. A side panel lists the APIs
   required to use Gemini in BigQuery.

4. For each required API, click **Enable** to enable the API for the current
   project, and then click **Next**.

5. To grant principals the IAM roles required to use
   Gemini in BigQuery, enter the user names of the principals.
   The following roles grant the permissions required to use Gemini:

   - [BigQuery Studio User](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioUser)
   - [BigQuery Studio Admin](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.studioAdmin)

   Gemini in BigQuery requires the following
   permissions:
   - **cloudaicompanion.entitlements.get**
   - **cloudaicompanion.instances.completeCode**
   - **cloudaicompanion.instances.completeTask**
   - **cloudaicompanion.instances.generateCode**
   - **cloudaicompanion.operations.get**
   - **cloudaicompanion.topics.create**
6. Click **Done**.

## Turn on Gemini in BigQuery features

If you're a data analyst, data scientist, or developer who wants to use
specific Gemini in BigQuery features to write
SQL queries and Python code, then you can toggle certain Gemini
features in the Google Cloud console. To learn how to toggle features, see
[Before you begin](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#before_you_begin) in
"Write queries with Gemini assistance." For more information, see
[Gemini for Google Cloud overview](https://docs.cloud.google.com/gemini/docs/overview).

### Use data insights and automated metadata generation features

BigQuery data insights and automated metadata generation features
are available to customers using BigQuery on-demand compute,
Enterprise edition, or Enterprise Plus edition. The quota for
data insights scans and metadata generation is based on the use of these
compute models at the
organization level. For information about quotas for these features, see
[Quotas for Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/quotas#bigquery).

If your organization is using BigQuery Standard edition
for compute only, then you can use Gemini Code Assist Standard, which
includes data insights and automated metadata generation capabilities in
addition to features listed in [Gemini Code Assist Standard and
Enterprise pricing
overview](https://cloud.google.com/products/gemini/pricing#gemini_code_assist_standard_and_enterprise_pricing_overview).
To learn how to purchase Gemini Code Assist Standard, see [Purchase a
Gemini Code Assist Standard
subscription](https://docs.cloud.google.com/gemini/docs/discover/set-up-gemini#purchase-subscription) and
follow the instructions to purchase Standard edition.

### Enable Gemini in BigQuery preview features

Certain Gemini in BigQuery features in
[Preview](https://cloud.google.com/products#product-launch-stages)
are part of the trusted tester program. To request access to these features,
an administrator must complete the
[Gemini in BigQuery Pre-GA Sign-up form](https://goo.gle/gemini-in-bq-preview).
Gemini in BigQuery pre-GA feature access is
enabled periodically in batches.

Preview features that require Gemini in BigQuery
sign-up include the following:

- Automated metadata generation for data insights (Preview)
- Dataset insights with BigQuery knowledge engine (Preview)

## Turn off Gemini in BigQuery

To prevent a user from using Gemini in BigQuery
features, revoke the specific `cloudaicompanion` IAM permissions
that grant access to these capabilities, as detailed in
[Enable necessary APIs and grant roles](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#enable-api).

To turn off specific Gemini in BigQuery
features:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2.
   In the toolbar, click
   pen_spark**Gemini settings**.

3. Clear the Gemini features that you want to turn off.

To turn off all Gemini for Google Cloud
products including BigQuery, see
[Turn off the Gemini for Google Cloud API](https://docs.cloud.google.com/gemini/docs/turn-off-gemini#companion-api).

## What's next

- Learn more about the [types of generative AI assistance available in Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/overview).
- Learn [where Gemini in BigQuery processes your data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).
- Learn [how to access and manage Gemini administrator controls](https://docs.cloud.google.com/gemini/docs/admin).
- Learn about [security, privacy, and compliance for Gemini in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/security-privacy-compliance).