# Introduction to developer tools

BigQuery provides a set of developer tools that you can use to
access BigQuery in your development environment, connect
BigQuery to external applications, and develop end-to-end
solutions. Before using these tools, you should be familiar with standard
BigQuery concepts, such as
[analysis](https://docs.cloud.google.com/bigquery/docs/query-overview) and
[resource organization](https://docs.cloud.google.com/bigquery/docs/resource-hierarchy).

## Tools for accessing BigQuery in your development environment

BigQuery APIs and client libraries are the core developer tools
for making BigQuery requests outside of the
Google Cloud console and bq command-line tool. When you access BigQuery in this
way, you must also provide some form of authentication.

### APIs

BigQuery offers
[REST and gRPC APIs](https://docs.cloud.google.com/bigquery/docs/reference/libraries-overview) to
programmatically interface with its various services. The following APIs are
available:

- [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/rest)
- [BigQuery Data Policy API](https://docs.cloud.google.com/bigquery/docs/reference/bigquerydatapolicy/rest)
- [BigQuery Connection API](https://docs.cloud.google.com/bigquery/docs/reference/bigqueryconnection/rest)
- [BigQuery Migration API](https://docs.cloud.google.com/bigquery/docs/reference/migration/rest)
- [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc)
- [BigQuery Reservation API](https://docs.cloud.google.com/bigquery/docs/reference/reservations/rest)
- [BigQuery Analytics Hub API](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest)
- [BigQuery Data Transfer Service API](https://docs.cloud.google.com/bigquery/docs/reference/datatransfer/rest)

### Client libraries

While you can use the BigQuery APIs directly by making requests
to the server, using the
[BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries)
can significantly reduce the amount of code that you need to write by providing
simplifications in your BigQuery API calls. The supported
languages for BigQuery are C#, Go, Java, Node.js, PHP, Python,
and Ruby. To try a quickstart for the BigQuery client libraries,
see
[Query a public dataset with the BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).

### Authentication

[Authentication](https://docs.cloud.google.com/docs/authentication) is the process by which your identity is
confirmed through the use of credentials. When you access
BigQuery in your development environment, a form of
authentication is always required. The most common authentication method for
BigQuery developers is
[Application Default Credentials](https://docs.cloud.google.com/bigquery/docs/authentication/getting-started#adc),
which automatically finds credentials based on your environment. For
more information on general authentication principles and other authentication
methods, see
[Authenticate to BigQuery](https://docs.cloud.google.com/bigquery/docs/authentication).

## Tools for connecting BigQuery to external applications

Several customized connection tools are available to help you incorporate
BigQuery capabilities with third-party applications.

### MCP Toolbox for Databases

Model Context Protocol (MCP) is an open protocol for connecting large language
models (LLMs) to data sources like BigQuery. The
[MCP Toolbox for Databases](https://docs.cloud.google.com/bigquery/docs/pre-built-tools-with-mcp-toolbox)
connects your BigQuery project to various Integrated Development
Environments (IDEs) and developer tools, empowering you build more powerful
AI agents with your BigQuery data.

### BigQuery agent analytics

BigQuery agent analytics is a first-party, open source solution
that captures, analyzes, and visualizes multimodal agent interaction data at
scale. This solution lets developers stream raw agent interactions, such as
requests, responses, tool calls, and errors directly into BigQuery.

To learn more about this solution, see [Use BigQuery
agent analytics](https://docs.cloud.google.com/bigquery/docs/bigquery-agent-analytics).

### ODBC and JDBC drivers

Open Database Connectivity (ODBC) and Java Database Connectivity (JDBC) drivers
connect applications to databases. Google partners with
[Simba](https://insightsoftware.com/simba/) to provide
[ODBC and JDBC drivers for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers),
which you can use to help build database-neutral software applications through
your preferred tooling and infrastructure. The
[Google-developed JDBC driver for BigQuery](https://docs.cloud.google.com/bigquery/docs/jdbc-for-bigquery)
is also available in
[Preview](https://cloud.google.com/products#product-launch-stages).

### Google Cloud for Visual Studio Code extension

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

If you're a Visual Studio Code (VS Code) user, you can use the
[Google Cloud VS Code extension](https://docs.cloud.google.com/bigquery/docs/vs-code-extension) to run
BigQuery notebooks and preview BigQuery datasets
from your existing VS Code environment.

## Tools for developing end-to-end solutions

As you build complex solutions with BigQuery, Google offers many
pathways to assist you, most notably, through code samples, repository and
workspace capabilities, and a wide variety of BigQuery
integrations.

### Code samples

[BigQuery code samples](https://docs.cloud.google.com/bigquery/docs/samples) provide
snippets for accomplishing common tasks in BigQuery, such as
creating tables, listing connections, viewing capacity commitments and
reservations, and loading data. You can use these code samples to start building
more complex solutions.

### Repositories and workspaces

> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

You can use [repositories](https://docs.cloud.google.com/bigquery/docs/repository-intro) to version control
the files that you use in BigQuery, and you can use
[workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces-intro) within those repositories to edit
code. BigQuery uses Git to record changes and manage file
versions. You can use the Git capabilities that are built into
BigQuery, or you can connect to a third-party Git repository.

### Integrated services and tools

The following Google services and tools integrate with BigQuery
and offer additional capabilities for building solutions:

- [**Managed Service for Apache Spark**](https://docs.cloud.google.com/dataproc/docs/concepts/overview). A fully managed service for running Apache Hadoop and Apache Spark jobs. Managed Service for Apache Spark provides the [BigQuery connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery), which lets Hadoop and Spark directly process data from BigQuery.
- [**Dataflow**](https://docs.cloud.google.com/dataflow/docs/about-dataflow). A fully managed service for running Apache Beam jobs at scale. The [BigQuery I/O connector for Beam](https://beam.apache.org/documentation/io/built-in/google-bigquery/) lets Beam pipelines read and write data to and from BigQuery.
- [**Managed Service for Apache Airflow**](https://docs.cloud.google.com/composer/docs/concepts/overview). A fully managed workflow scheduling service built on Apache Airflow. [BigQuery operators](https://airflow.apache.org/docs/apache-airflow-providers-google/stable/operators/cloud/bigquery.html) let Airflow workflows manage datasets and tables, run queries, and validate data.
- [**Pub/Sub**](https://docs.cloud.google.com/pubsub/docs/overview). An asynchronous and scalable messaging service. Pub/Sub provides [BigQuery subscriptions](https://docs.cloud.google.com/pubsub/docs/bigquery), which you can use for writing messages to an existing BigQuery table as they are received.
- [**Dataform**](https://docs.cloud.google.com/dataform/docs/overview). A service for data analysts to develop, test, version control, and schedule complex SQL workflows for data transformation in BigQuery.
- [**BigQuery Terraform module**](https://github.com/terraform-google-modules/terraform-google-bigquery/blob/master/README.md). A module to automate the instantiation and deployment of your BigQuery datasets and tables.
- [**bq command-line tool**](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool). A Python-based command-line tool for BigQuery.

Google also validates dozens of partner solutions and integrations for
BigQuery through the
[Google Cloud Ready - BigQuery](https://docs.cloud.google.com/bigquery/docs/bigquery-ready-overview)
program. These recognized partners have met a core set of requirements to ensure
compatibility with BigQuery.

## What's next

- For information about resources and upcoming events for Google Cloud developers, visit the [developer center](https://cloud.google.com/developers).
- For information about how other companies use Google Cloud, see [Data Cloud for ISVs](https://cloud.google.com/solutions/data-cloud-isvs).