# Schedule workloads

BigQuery tasks are usually part of larger workloads, with external
tasks triggering and then being triggered by BigQuery operations.
Workload scheduling helps data administrators, analysts, and developers
organize and optimize this chain of actions, creating a seamless connection
across data resources and processes. Scheduling methods and tools assist
in designing, building, implementing, and monitoring these complex data
workloads.

## Choose a scheduling method

To select a scheduling method, you should identify whether your workloads
are event-driven, time-driven, or both. An *event* is defined as a state change,
such as a change to data in a database or a file added to a storage system. In
*event-driven scheduling* , an action on a website might trigger a data
activity, or an object landing in a certain bucket might need to be processed
immediately on arrival. In *time-driven scheduling*, new data might need to
be loaded once per day or frequently enough to produce hourly reports. You can
use event-driven and time-driven scheduling in scenarios where you need to
load objects into a data lake in real time, but activity reports on the data
lake are only generated daily.

## Choose a scheduling tool

Scheduling tools assist with tasks that are involved in managing complex data
workloads, such as combining multiple Google Cloud or third-party services with
BigQuery jobs, or running multiple BigQuery jobs
in parallel. Each workload has unique requirements for dependency and parameter
management to ensure that tasks are executed in the correct order using the
correct data. Google Cloud provides several scheduling options that are
based on scheduling method and workload requirements.

We recommend using Dataform, Workflows,
Managed Airflow, or Vertex AI Pipelines for most use cases.
Consult the following chart for a side-by-side comparison:

|   | [Dataform](https://docs.cloud.google.com/bigquery/docs/orchestrate-workloads#dataform) | [Workflows](https://docs.cloud.google.com/bigquery/docs/orchestrate-workloads#workflows) | [Managed Airflow](https://docs.cloud.google.com/bigquery/docs/orchestrate-workloads#composer) | [Vertex AI Pipelines](https://docs.cloud.google.com/bigquery/docs/orchestrate-workloads#vertex) |
| Focus | Data transformation | Microservices | ETL or ELT | Machine learning |
| Complexity | \* | \*\* | \*\*\* | \*\* |
| User profile | Data analyst or administrator | Data architect | Data engineer | Data analyst |
| Code type | JavaScript, SQL, [Python notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks) | YAML or JSON | Python | Python |
| Serverless? | Yes | Yes | Fully managed | Yes |
| Not suitable for | Chains of external services | Data transformation and processing | Low latency or event-driven pipelines | Infrastructure tasks |
|---|---|---|---|---|

The following sections detail these scheduling tools and several others.

### Scheduled queries

The simplest form of workload scheduling is
[scheduling recurring queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries) directly in
BigQuery. While this is the least complex approach to
scheduling, we recommend it only for straightforward query chains with no
external dependencies. Queries scheduled in this way must be written in
[GoogleSQL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax) and
can include
[data definition language (DDL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language)
and
[data manipulation language (DML)](https://docs.cloud.google.com/bigquery/docs/data-manipulation-language)
statements.

**Scheduling method**: time-driven

### Dataform

[Dataform](https://docs.cloud.google.com/dataform/docs/overview) is a free, SQL-based, opinionated
transformation framework that schedules complex data transformation tasks in
BigQuery. When raw data is loaded into BigQuery,
Dataform helps you create an organized, tested,
version-controlled collection of datasets and tables. Use
Dataform to schedule runs for your
[data preparations](https://docs.cloud.google.com/bigquery/docs/orchestrate-data-preparations),
[notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks),
and [BigQuery pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).

**Scheduling method**: time-driven

> [!NOTE]
> **Note:** If you create an asset in a BigQuery repository---for example, a query, notebook (including a notebook with an Apache Spark job), BigQuery pipeline, or Dataform workflow---you cannot schedule it for execution in Dataform. Instead, you need to use BigQuery execution and scheduling capabilities. For more information, see [Scheduling queries](https://docs.cloud.google.com/bigquery/docs/scheduling-queries), [Schedule notebooks](https://docs.cloud.google.com/bigquery/docs/orchestrate-notebooks), and [Schedule pipelines](https://docs.cloud.google.com/bigquery/docs/schedule-pipelines).

### Workflows

[Workflows](https://docs.cloud.google.com/workflows/docs/overview) is a serverless tool that
schedules HTTP-based services with very low latency. It is best for chaining
microservices together, automating infrastructure tasks, integrating with
external systems, or creating a sequence of operations in Google Cloud.
To learn more about using Workflows with BigQuery,
see
[Run multiple BigQuery jobs in parallel](https://docs.cloud.google.com/workflows/docs/tutorials/bigquery-parallel-jobs).

**Scheduling method**: event-driven and time-driven

### Managed Service for Apache Airflow

[Managed Airflow](https://docs.cloud.google.com/composer/docs/concepts/overview) is a fully managed
tool built on Apache Airflow. It is best for extract, transform, load (ETL) or
extract, load, transform (ELT) workloads as it supports several
[operator](https://airflow.apache.org/docs/apache-airflow/stable/concepts/operators.html)
types and patterns, as well as task execution across other Google Cloud
products and external targets. To learn more about using Managed Airflow
with BigQuery, see
[Run a data analytics DAG in Google Cloud](https://docs.cloud.google.com/composer/docs/data-analytics-googlecloud).

**Scheduling method**: time-driven

### Vertex AI Pipelines

[Vertex AI Pipelines](https://docs.cloud.google.com/vertex-ai/docs/pipelines/introduction) is a
serverless tool based on Kubeflow Pipelines specially designed for scheduling
machine learning workloads. It automates and connects all tasks of your model
development and deployment, from training data to code, giving you a complete
view of how your models work. To learn more about using
Vertex AI Pipelines with BigQuery, see
[Export and deploy a BigQuery machine learning model for prediction](https://codelabs.developers.google.com/codelabs/bqml-vertex-prediction#0).

**Scheduling method**: event-driven

### Apigee Integration

[Apigee Integration](https://docs.cloud.google.com/apigee/docs/api-platform/integration/what-is-apigee-integration)
is an extension of the Apigee platform that includes connectors and
data transformation tools. It is best for integrating with external enterprise
applications, like Salesforce. To learn more about using
Apigee Integration with BigQuery, see
[Get started with Apigee Integration and a Salesforce trigger](https://docs.cloud.google.com/apigee/docs/api-platform/integration/getting-started-salesforce-updates).

**Scheduling method**: event-driven and time-driven

### Cloud Data Fusion

[Cloud Data Fusion](https://docs.cloud.google.com/data-fusion) is a data integration tool that
offers code-free ELT/ETL pipelines and over 150 preconfigured connectors and
transformations. To learn more about using Cloud Data Fusion with
BigQuery, see
[Replicating data from MySQL to BigQuery](https://docs.cloud.google.com/data-fusion/docs/tutorials/replicating-data/mysql-to-bigquery).

**Scheduling method**: event-driven and time-driven

### Cloud Scheduler

[Cloud Scheduler](https://docs.cloud.google.com/scheduler/docs/overview) is a fully managed
scheduler for jobs like batch streaming or infrastructure operations that should
occur on defined time intervals. To learn more about using
Cloud Scheduler with BigQuery, see
[Scheduling workflows with Cloud Scheduler](https://docs.cloud.google.com/scheduler/docs/tut-workflows).

**Scheduling method**: time-driven

### Cloud Tasks

[Cloud Tasks](https://docs.cloud.google.com/tasks/docs/dual-overview) is a fully managed
service for asynchronous task distribution of jobs that can execute
independently, outside of your main workload. It is best for delegating slow
background operations or managing API call rates. To learn more
about using Cloud Tasks with BigQuery, see
[Add a task to a Cloud Tasks queue](https://docs.cloud.google.com/tasks/docs/add-task-queue).

**Scheduling method**: event-driven

### Third-party tools

You can also connect to BigQuery using a number of
popular third-party tools such as CData and SnapLogic. The
BigQuery Ready program offers a
[full list of validated partner solutions](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-partners).

## Messaging tools

Many data workloads require additional messaging connections between decoupled
microservices that only need to be activated when certain events occur.
Google Cloud provides two tools that are designed to integrate with
BigQuery.

### Pub/Sub

[Pub/Sub](https://docs.cloud.google.com/pubsub/docs/overview) is an asynchronous messaging tool
for data integration pipelines. It is designed to ingest and distribute data
like server events and user interactions. It can also be used for parallel
processing and data streaming from IoT devices. To learn more about using
Pub/Sub with BigQuery, see
[Stream from Pub/Sub to BigQuery](https://docs.cloud.google.com/dataflow/docs/tutorials/dataflow-stream-to-bigquery).

### Eventarc

[Eventarc](https://docs.cloud.google.com/eventarc/docs/overview) is an event-driven tool that
lets you manage the flow of state changes throughout your data pipeline. This
tool has a wide range of use cases including automated error remediation,
resource labeling, image retouching, and more. To learn more about using
Eventarc with BigQuery, see
[Build a BigQuery processing pipeline with Eventarc](https://docs.cloud.google.com/eventarc/docs/run/bigquery).

## What's next

- Learn to [schedule recurring queries directly in BigQuery](https://docs.cloud.google.com/bigquery/docs/scheduling-queries).
- Get started with [Dataform](https://docs.cloud.google.com/dataform/docs/overview).
- Get started with [Workflows](https://docs.cloud.google.com/workflows/docs/overview).
- Get started with [Managed Airflow](https://docs.cloud.google.com/composer/docs/concepts/overview).
- Get started with [Vertex AI Pipelines](https://docs.cloud.google.com/vertex-ai/docs/pipelines/introduction).
- Get started with [Apigee Integration](https://docs.cloud.google.com/apigee/docs/api-platform/integration/what-is-apigee-integration).
- Get started with [Cloud Data Fusion](https://docs.cloud.google.com/data-fusion).
- Get started with [Cloud Scheduler](https://docs.cloud.google.com/scheduler/docs/overview).
- Get started with [Pub/Sub](https://docs.cloud.google.com/pubsub/docs/overview).
- Get started with [Eventarc](https://docs.cloud.google.com/eventarc/docs/overview).