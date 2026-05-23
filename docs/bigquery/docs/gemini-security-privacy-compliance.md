# Security, privacy, and compliance for Gemini in BigQuery

This document describes the controls that support the security of Gemini in BigQuery. These controls can also help you meet the privacy and regulatory requirements that apply to your business. Gemini in BigQuery is built on Google Cloud infrastructure. Your data remains in your control. For more information, see [Service Specific Terms](https://cloud.google.com/terms/service-terms).

<br />

The following controls apply to Generally Available (GA) Gemini
in BigQuery features:

- **Your data is not used for training models without your permission.** Google does not use your prompts, responses, or schema information to train its models unless you explicitly opt in.
- **Your BigQuery data remains within your chosen location.** Gemini in BigQuery respects your BigQuery data at rest data residency settings. The core BigQuery engine that runs queries and stores your data continues to honor your location constraints. For more information, see [How Gemini in BigQuery processes data](https://docs.cloud.google.com/bigquery/docs/gemini-security-privacy-compliance#how-gemini-processes-data).
- **Gemini in BigQuery is covered by Google
  security and compliance offerings.** Coverage includes certifications like SOC 1/2/3, ISO/IEC 27001, and [HIPAA compliance](https://cloud.google.com/security/compliance/hipaa). For more information, see [Google security and compliance offerings](https://cloud.google.com/security/compliance/offerings).

The security, privacy, and compliance for Google Cloud services are a
[shared responsibility](https://docs.cloud.google.com/architecture/framework/security/shared-responsibility-shared-fate).
Google secures the infrastructure that Google Cloud services run on, and it
provides you with tools such as access controls to let you manage who has access
to your services and resources. For more information about how the infrastructure is secured, see
[Google infrastructure security design overview](https://docs.cloud.google.com/docs/security/infrastructure/design).

Because Gemini is an evolving technology, it can generate output
that's plausible-sounding but factually incorrect. We recommend that you
validate all output from Gemini before you use it. For more
information, see [Gemini for Google Cloud and responsible
AI](https://docs.cloud.google.com/gemini/docs/discover/responsible-ai).

### Gemini in BigQuery architecture

The following diagram shows the components of the Gemini in
BigQuery architecture.

![Chart of Gemini in BigQuery global and EU and US
jurisdictions.](https://docs.cloud.google.com/static/bigquery/images/ginbq-region-jurisdictions.svg)

> [!IMPORTANT]
> **Important:** Gemini in BigQuery processes data in the `US` or `EU` jurisdictions where the data resides. Data outside these jurisdictions is processed globally. To learn more about where Gemini in BigQuery processes your data, see [Where Gemini in BigQuery processes your data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).

### How Gemini in BigQuery processes data

When a user uses Gemini in BigQuery, a prompt and
its relevant context are sent to Google's large language models (LLMs) for
processing. Google manages the specific models used to generate
Gemini in BigQuery responses.

1. **Prompt.** A user enters a prompt as a natural language question, such as "Show me the top 5 customers by sales last quarter". Or, a user types a partial SQL or Python snippet in the Google Cloud console in BigQuery Studio with Gemini in BigQuery enabled.
2. **Contextualization.** Gemini in BigQuery accesses the relevant metadata and schema of your BigQuery tables to add context to the user's prompt. Contextual information can include sampling data from tables and job histories. Gemini in BigQuery only has access to the resources to which the user has access.
3. **Gemini processing.** The prompt and contextual information are sent to Gemini's LLMs for processing. Gemini in BigQuery doesn't retain or store contextual information. Gemini in BigQuery uses the existing BigQuery context that is stored in Knowledge Catalog and Spanner. This information resides in the same location as your data. Gemini generates a response, such as a SQL query, a data insight, or a Python code snippet.
4. **Response.** The response is returned to the BigQuery interface. The user can then run the generated code, modify it, or continue to iterate on the response by using Gemini. You can provide feedback from Gemini in BigQuery in the Google Cloud console. To learn more about providing feedback, see [Provide feedback](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#provide_feedback_2).

## Security controls

Gemini in BigQuery uses the security controls of
Google Cloud to help protect your data and resources. These controls
include the following:

- **Authentication.** Users authenticate by using their Google Cloud credentials, which can be integrated with your existing identity provider.
- **Access controls.** You can use Identity and Access Management (IAM) to control who has access to Gemini in BigQuery and what actions they can perform.
- **Network security and VPC-SC.** Gemini in BigQuery traffic is encrypted in transit and at rest. You can also use [VPC Service Controls](https://docs.cloud.google.com/bigquery/docs/vpc-sc) to create a security-enhanced perimeter around your BigQuery resources.

## Data protection and privacy

Gemini in BigQuery is designed to protect the
privacy of your data. Google's privacy policies and commitments apply to all
data processed by Gemini in BigQuery.

- **Data encryption.** Your data is encrypted at rest and in transit.
- **Data access.** Google personnel have limited and audited access to your data.
- **Data residency.** Your BigQuery data-at-rest is stored and processed in the Google Cloud region you select.

## Certifications and capabilities

Generally available (GA) Gemini in BigQuery
features are covered by the certifications and security statements of
Gemini for Google Cloud with exception of the
following limitations:

- Gemini in BigQuery doesn't provide data residency for individual locations. Gemini processing can be specified for data with the `US`- and `EU`-supported jurisdictions. Data outside these jurisdictions is processed globally. To learn more, see [Where Gemini in BigQuery processes your
  data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).
- Cloud logging audit logs are not available for Gemini in BigQuery user prompts and responses.
- Gemini in BigQuery is not included in supported [Assured Workload packages](https://docs.cloud.google.com/assured-workloads/docs/supported-products).

To learn more about certifications and security for Gemini for
Google Cloud, see
[certifications and security for Gemini for
Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications).

## Secure and responsible use

You should adhere to the following best practices to help ensure the secure and
responsible use of Gemini in BigQuery:

- Use IAM to give the least privilege necessary. For information about security best practices in BigQuery, see [Introduction to security and access controls in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/access-control-intro).
- Be mindful of the data you include in your natural language prompts in BigQuery, such as sensitive or personal information.
- Review and validate the responses generated by Gemini in BigQuery. Always treat AI-generated code and analysis as suggestions that require human review.
- Only enable Gemini in BigQuery for projects that don't require compliance offerings other than those listed previously and by [Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications). For information about how to turn off or prevent access to Gemini in BigQuery, see [Turn off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off).

## What's next

- Read [Overview for Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-overview).
- Read [Setup for
  Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).
- Read [Introduction to data governance in
  BigQuery](https://docs.cloud.google.com/bigquery/docs/data-governance).
- Read [Certifications and security for Gemini for
  Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications).
- Read [Supported products by control package](https://docs.cloud.google.com/assured-workloads/docs/supported-products).
- Read [How Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).