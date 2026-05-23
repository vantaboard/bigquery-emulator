# Gemini in BigQuery overview

[Video](https://www.youtube.com/watch?v=-MWIHAH4cbA)

This document describes how Gemini in BigQuery, which is part
of the [Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/overview) product suite,
provides AI-powered assistance to help you work with your data.

## AI assistance with Gemini in BigQuery

Gemini in BigQuery provides AI assistance to help
you do the following:

- **Explore and understand your data with data insights** . Data insights offers an automated, intuitive way to uncover patterns and perform statistical analysis by using insightful queries that are generated from the metadata of your tables. This feature is especially helpful in addressing the cold-start challenges of early data exploration. For more information, see [Generate data insights in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-insights).
- **Discover, transform, query, and visualize data with BigQuery data canvas** . You can use natural language with Gemini in BigQuery, to find, join, and query table assets, visualize results, and seamlessly collaborate with others throughout the entire process. For more information, see [Analyze with
  data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas).
- **Get assisted SQL and Python data analysis** . You can use Gemini in BigQuery to generate or suggest code in either SQL or Python, and to explain an existing SQL query. You can also use natural language queries to begin data analysis. To learn how to generate, complete, and summarize code, see the following documentation:  
  - SQL code assist
    - [Use the SQL generation tool](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#use_the_sql_generation_tool)
    - [Prompt to generate SQL queries](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#chat)
    - [Generate SQL queries with Gemini Cloud Assist](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#chat) ([Preview](https://cloud.google.com/products#product-launch-stages))
    - [Convert comments to SQL](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#natural_language) ([Preview](https://cloud.google.com/products#product-launch-stages))
    - [Complete a SQL query](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#complete_a_sql_query) ([Preview](https://cloud.google.com/products#product-launch-stages))
    - [Explain a SQL query](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#explain_a_sql_query)
  - Python code assist
    - [Generate Python code with the code generation tool](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_python_code)
    - [Generate Python code with Gemini Cloud Assist](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#chat-python) ([Preview](https://cloud.google.com/products#product-launch-stages))
    - [Python code completion](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#complete_python_code)
    - [Generate BigQuery DataFrames Python code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#dataframe) ([Preview](https://cloud.google.com/products#product-launch-stages))
- **Prepare data for analysis** . Data preparation in BigQuery gives you context aware, AI-generated transformation recommendations to cleanse data for analysis. For more information, see [Prepare data with Gemini](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions).
- **Customize your SQL translations with translation rules** . ([Preview](https://cloud.google.com/products#product-launch-stages)) Create Gemini-enhanced translation rules to customize your SQL translations when using the [interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator). You can describe changes to the SQL translation output using natural language prompts or specify SQL patterns to find and replace. For more information, see [Create a translation
  rule](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator#create-apply-rules).

Gemini for Google Cloud doesn't use your prompts or its
responses as data to train its models without your express permission. For more
information about how Google uses your data, see
[How Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance).

> [!CAUTION]
> As an early-stage technology, Gemini for Google Cloud
> products can generate output that seems plausible but is factually incorrect. We recommend that you
> validate all output from Gemini for Google Cloud products before you use it.
> For more information, see
> [Gemini for Google Cloud and responsible AI](https://docs.cloud.google.com/gemini/docs/discover/responsible-ai).

For information about security, privacy and compliance, see
[Security, privacy, and compliance for Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-security-privacy-compliance).

> [!NOTE]
> **Note** : Gemini in BigQuery is part of Gemini for Google Cloud and doesn't support the same compliance and security offerings as BigQuery. You should only set up Gemini in BigQuery for BigQuery projects that don't require [compliance offerings that aren't supported by Gemini for Google Cloud](https://docs.cloud.google.com/gemini/docs/discover/certifications). For information about how to turn off or prevent access to Gemini in BigQuery, see [Turn off Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up#turn-off).

## Pricing

See [Gemini for Google Cloud pricing](https://cloud.google.com/products/gemini/pricing).

## Where to interact with Gemini in BigQuery

After you [set up Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up),
you can use Gemini in BigQuery to do the following
in BigQuery Studio:

- To [generate data insights](https://docs.cloud.google.com/bigquery/docs/generate-table-insights#insights-bigquery-table), go to the **Insights** tab for a table entry, where you can identify patterns, assess quality, and run statistical analysis across your BigQuery data.
- To use data canvas, [create a data canvas or use data canvas](https://docs.cloud.google.com/bigquery/docs/data-canvas#work-with-data-canvas) from a table or query to explore data assets with natural language and share your canvases.
- To use natural language to generate [SQL
  queries](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_a_sql_query) or [Python
  code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#complete_python_code), use [comments
  in code](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#generate_sql_from_a_comment) or the [SQL generation
  tool](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#use_the_sql_generation_tool). You can also receive [suggestions with autocomplete while
  typing](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini#complete_a_sql_query).
- To prepare data for analysis, in the **Create new** list, select **Data preparation** . For more information, see [Open the data preparation editor in BigQuery](https://docs.cloud.google.com/bigquery/docs/data-prep-get-suggestions#open-data-prep-editor).

## Set up Gemini in BigQuery

For detailed setup steps, see
[Set up Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).

## How Gemini in BigQuery uses your data

In order to provide accurate results, Gemini in
BigQuery requires access to both your
[Customer Data](https://cloud.google.com/terms/data-processing-addendum) and metadata
in BigQuery for enhanced features. Enabling Gemini
in BigQuery grants Gemini permission to access
this data, which includes your tables and query history. Gemini
in BigQuery doesn't use your data to train or fine-tune its
models. For more information on how Gemini uses your data, see
[Security, privacy, and compliance for Gemini
in BigQuery](https://docs.cloud.google.com/gemini/docs/bigquery/security-privacy-compliance).

Enhanced features in Gemini in BigQuery are the following:

- SQL generation tool
- Prompt to generate SQL queries
- Convert comments to SQL
- Complete a SQL query
- Explain a SQL query
- Generate python code
- Python code completion
- Data canvas
- Data preparation
- Data insights

### Locations

For information about where Gemini in BigQuery processes
your data, see [Where Gemini in BigQuery processes
your data](https://docs.cloud.google.com/bigquery/docs/gemini-locations).

## What's next

- Learn how to [set up Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-set-up).
- Learn how to [write queries with Gemini assistance](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini).
- Learn more about [Google Cloud compliance](https://cloud.google.com/security/compliance).
- Learn about [security, privacy, and compliance for Gemini in BigQuery](https://docs.cloud.google.com/bigquery/docs/gemini-security-privacy-compliance).
- Learn more about [how Gemini for Google Cloud uses your data](https://docs.cloud.google.com/gemini/docs/discover/data-governance)