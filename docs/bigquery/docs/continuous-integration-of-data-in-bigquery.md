<br />

This document describes principles and techniques for implementing a repeatable
workflow that will help you integrate data changes into your
BigQuery-based data warehouse (DWH). These changes might include
new datasets, new data sources, or updates and changes to existing datasets. The
document also describes a reference architecture for this task.

The audience for this document is software and data architects and data
engineers who use BigQuery as a DWH. The document assumes that
you're familiar with basic concepts of CI/CD or similar application lifecycle
management practices.

## Introduction

Continuous integration and continuous delivery (CI/CD) has become an essential
technique in the software development lifecycle. Adopting the principles of
CI/CD lets teams deliver better software with fewer issues than by integrating
features and deploying them manually. CI/CD can also be part of a strategy for
data management when you modernize your data warehousing.

However, when you're working with a DWH like BigQuery, there are
differences in how you implement CI/CD compared to implementing CI/CD in source
code. These differences are in part because data warehousing is an
inherently stateful system for managing the underlying data.

This document provides the following information:

- Techniques for implementing a continuous integration (CI) strategy in BigQuery.
- Guidance and methods that help you avoid pitfalls.
- Suggestions for BigQuery features that help with CI in BigQuery.

This document focuses on CI, because integration has more data-specific
considerations for a data warehousing team than continuous delivery (CD) does.

### When to use CI for a BigQuery DWH

In this document, *data integration* is a task that's usually performed by
the DWH team, which includes incorporating new data into the DWH. This task
might involve incorporating a new data source into the DWH, or changing the
structure of a table that's already inside the DWH.

Integrating new data into the DWH is a task similar to integrating a new
feature into existing software. It might introduce bugs and negatively affect
the end-user experience. When you integrate data into BigQuery,
downstream consumers of the data (for example, applications, BI dashboards, and
individual users) might experience issues due to schema mismatches.
Or the consumers might use incorrect data that doesn't reflect the data from
the original data source.

CI for DWH is useful when you want to do the following:

- Describe key points in CI for a DWH system.
- Design and implement a CI strategy for your BigQuery environment.
- Learn how to use BigQuery features for implementing CI.

This guide doesn't describe how to manage CI for non-DWH products, including
data products like Dataflow and Bigtable.

### Example scenario

Example Company is a large retail company that maintains its DWH in
BigQuery. In the upcoming year, the company wants to integrate
new data sources into its DWH from companies that were recently acquired by
Example Company. The new data sources to be integrated have different schemas.
However, the DWH must keep its existing schema and must provide strong data
consistency and data completeness so that the downstream consumers of data
aren't negatively affected.

Currently, the Example Company DWH team performs data integration. The
integration relies on having the existing data sources in a predefined schema.
This workflow involves legacy data import processes, acquired databases, and
application services.

To update their data integration processes to accommodate the new data sources,
the DWH team must redesign their approach to data integration to comply with
the requirements that were noted earlier, such as strong data consistency.
The team must implement the changes in an isolated fashion so that the
data changes can be tested and measured before the data is made available to
downstream consumers.

After the DWH team adopts the new workflow, the team has a repeatable
process. Each developer can create an isolated development environment that's
based on production data. Using these isolated environments, developers can then
make changes, test them, have them reviewed, and deliver the required changes to
the production environment. If the changes cause bugs or unforeseen
issues, the changes can be easily rolled back.

## What continuous integration means for a DWH

[Continuous integration (CI)](https://en.wikipedia.org/wiki/Continuous_integration)
is a set of practices that lets development teams shorten development cycles and
find issues in the code faster than with manual systems. The main benefit of
adopting a CI approach is the ability to develop rapidly, reducing the risks of
interference between developers. This goal is achieved by making sure that the
process is repeatable, while allowing each developer to work in isolation from
other developers.

These principles also apply when an organization must integrate data into a
DWH, with a few differences. In the context of typical software development, CI
isolates changes to source code, which is stateless. In the context of CI in
data, CI integrates data into a DWH system. However, data is stateful by
definition. This difference has implications for how CI applies to DWH
scenarios, as described in this document.

### Additional scenarios that aren't covered in this document

Although this document focuses on isolating development changes from the
production environment, the document doesn't cover the following aspects of data
integration:

- **Data testing:** Are you able to verify that the data you have conforms to business requirements? Is the data reliable to serve as the source of truth? To increase your confidence level in the data that you're serving from your DWH, it's important to test the data. To test, you can run a set of queries, asserting that the data isn't missing values or asserting that it contains "bad" values.
- **Data lineage:** Are you able to see any table in its context? For example, can you see where the data was gathered from, and which datasets were pre-computed in order to generate the table? In modern DWH architectures, data is split into many systems that use different, specialized data structures. These include relational databases, NoSQL databases, and external data sources. To fully understand the data that you have, you must keep track of that data. You must also understand how the data was generated and from where it was generated.

These topics are out of scope for this guide. However, it will benefit your
data strategy to plan for these topics when you're designing a workflow for your
team.

## Typical setup of BigQuery as a DWH

The following diagram illustrates a typical DWH design for
BigQuery. It shows how data from external sources is ingested
into the DWH, and how consumers consume data from the DWH.

![Three databases outside of Google Cloud are data sources. Their data
moves into storage in a staging area. The data then moves into
BigQuery tables, which are the source for
BigQuery views. Consumers like Looker, App Engine,
Vertex AI notebooks, and human users consume the data using the views.](https://docs.cloud.google.com/static/bigquery/images/continuous-integration-of-data-in-bigquery-typical-dwh-design.svg)

The data starts at the data sources, where the data is in conventional
transactional or low-latency databases such as OLTP SQL databases and NoSQL
databases. A scheduled process copies the data into a staging area.

The data is stored temporarily in the staging area. If necessary, the data is
transformed to fit an analytical system before it's loaded into the DWH tables.
(In this diagram, the staging area is inside Google Cloud, but it doesn't have
to be.) Transformations might include denormalization, enriching certain
datasets, or handling malformed entries (for example, entries with missing
values).

From the staging area, the new data is loaded into the DWH tables. The tables
might be organized in different ways depending on the design of the DWH, and are
usually referred to as *core tables* . Some examples of table design paradigms
include the
[star schema](https://en.wikipedia.org/wiki/Star_schema)
paradigm, the
[denormalized](https://en.wikipedia.org/wiki/Denormalization)
paradigm, and
[multi-tier aggregates](https://en.wikipedia.org/wiki/Online_analytical_processing#Multidimensional_databases).

Regardless of the table design, these tables save data over time. The tables
must adhere to the schema, and they're presumed to hold the source of truth for
all analytical purposes. This role for the tables means that data in these
tables must be complete, be consistent, and adhere to the predefined schemas.

These tables don't serve data directly to consumers. Instead, the data is
served through an access layer, which is designed to encapsulate business logic
that must be applied to the underlying data. Examples of this type of business
logic are calculating a metric for each record, or filtering and grouping
the data.

The consumers of the data can connect to and read data from the DWH access
layer. These data consumers might include systems like the following:

- Business intelligence (BI) dashboards
- Data science notebooks
- Operational systems that rely on data calculated in the DWH
- Human users for ad-hoc queries

The data consumers rely heavily on the DWH for providing consistent schemas and
on the business logic that the DWH encapsulates. These schemas and business
logic can be considered as the service level agreements (SLAs) of the DWH
platform. Any change to the business logic, to the schema, or to the
completeness of data might have large implications downstream. Given the
ever-changing nature of modern data platforms, the DWH team might be required to
make those changes while nevertheless strictly adhering to the SLAs. In order
for the team to meet these SLAs and also keep the DWH up to date, they need a
workflow that allows data integration while minimizing the friction that these
changes might create.

## Assets for continuous integration in a DWH

As with any other development or IT team, the DWH team must maintain assets
that are essential to their responsibilities. These assets can typically be
divided into the following categories:

- **The codebase for data pipelines**: These assets usually consist of
  source code in a high-level programming language like Python or Java. For
  those types of assets, the CI/CD processes are built by using
  tools like Git and Jenkins, or by using
  Google Cloud solutions like Cloud Source Repositories and Cloud Build.

- **SQL scripts**: These assets describe the structure and the business
  logic that's encapsulated inside the DWH. Within this category, the assets
  can be further divided into the following subcategories:

  - **Data definition language (DDL)**: These assets are used for defining the schema of tables and views.
  - **Data manipulation language (DML)**: These assets are used for manipulating data inside a table. DML commands are also used to create new tables based on existing tables.
  - **Data control language (DCL)** : These assets are used for controlling permissions and access to tables. Within BigQuery, you can control access by using SQL and the [`bq`](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool) command-line tool or by using the BigQuery REST API. However, we recommend that you use IAM.

These assets, and others like Terraform scripts that are used to build
components, are maintained inside code repositories. Tools like
[Dataform](https://dataform.co/)
can help you construct a CI/CD pipeline that validates your SQL scripts and
checks predefined validation rules on tables that are created by DDL scripts.
These tools let you apply compilation and testing processes for SQL, which in
most contexts doesn't have a natural testing environment.

In addition to the assets that are associated with tools and processes, the
main asset for a DWH team is the data. Data isn't trackable by using
asset-tracking systems like Git, which is designed to track source code. This
document addresses the issues that are associated with tracking data.

## Issues with integrating data

Because of the potential complexity of table relationships inside a DWH (for
example, in one of the table design paradigms mentioned earlier), keeping the
state of production data isolated from a testing environment is a challenge.
Standard practices in software development can't be applied to the data
integration scenario.

The following table summarizes the differences between the practices for
integrating code and the practices for integrating data.

|   | Integrating code | Integrating data |
|---|---|---|
| Local development | Source code is easily cloneable due to its relatively small size. Generally the code fits most end-user machines (excluding cases of monorepos, which have other solutions). | Most tables in a DWH cannot fit on a development machine due to their size. |
| Central testing | Different states of the source code are cloned into a central system (a CI server) to undergo automated testing. Having different states of the code lets you compare results between a stable version and a development version. | Creating different states of the data in an isolated environment isn't straightforward. Moving data outside the DWH is a resource-intensive and time-consuming operation. It isn't practical to do as frequently as needed for testing. |
| Past versions | During the process of releasing new versions of software, you can track past versions. If you detect a problem in a new release, you can roll back to a safe version. | Taking backups of tables inside the DWH is a standard practice in case you must roll back. However, you must make sure that all affected tables are rolled back to the same point in time. That way, related tables are consistent with one another. |

## Integrate data into BigQuery tables

BigQuery has two features that can help you design a workflow
for data integration:
[table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro)
and
[table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro).
You can combine these features to achieve a workflow that gives you a
cost-effective development environment. Developers can manipulate data and its
structure in isolation from the production environment, and they can roll back a
change if necessary.

A BigQuery table snapshot is a read-only representation of a
table (called the *base table*) at a given moment in time. Similarly, A
BigQuery table clone is a read-write representation of a table
at a given moment in time. In both cases, storage costs are minimized because
only the differences from the base table are stored. Table clones start to incur
costs when the base table changes or when the table clones change. Because
table snapshots are read-only, they incur costs only when the base table
changes.

For more information about the pricing of table snapshots and table clones, see
[Introduction to table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro#storage_costs)
and
[Introduction to table clones](https://docs.cloud.google.com/bigquery/docs/table-clones-intro).

You can take table snapshots and table clones using the BigQuery
[time travel](https://docs.cloud.google.com/bigquery/docs/time-travel)
feature (up to seven days in the past). This feature lets you capture snapshots
and clones of multiple tables at the same point in time, which makes your
working environment and snapshots consistent with one another. Using this feature
can be helpful for tables that are updated frequently.

### How to use table clones and table snapshots to allow isolation

To illustrate the integration workflow for CI in a DWH, imagine the following
scenario. You're given a task of integrating a new dataset into the DWH. The
task might be to create new DWH tables, to update existing tables, to change the
structure of tables, or any combination of these tasks. The workflow might look
like the following sequence:

1. You identify the tables that might be affected by the changes and additional tables that you might want to check.
2. You [create a new BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets) to contain the assets for this change. This dataset helps isolate the changes and separates this task from other tasks that other team members work on. The dataset must be in the same region as the source dataset. However, the project can be separated from the production project to help with your organization's security and billing requirements.
3. For each of the tables, you create both a
   [clone](https://docs.cloud.google.com/bigquery/docs/table-clones-create)
   and a
   [snapshot](https://docs.cloud.google.com/bigquery/docs/table-snapshots-create)
   in the new dataset, potentially for the same point in time. This approach
   offers the following benefits:

   - The table clone can act as a working copy where you can make changes freely without affecting the production table. You can create multiple table clones of the same base table in order to test different integration paths at the same time, with minimal overhead.
   - The snapshot can act as a restore and reference point, a point where the data is known to have worked before any change took place. Having this snapshot lets you perform a rollback in case an issue is detected later in the process.
4. You use the table clones to implement the changes that are required for
   the tables. This action results in an updated version of the table clones,
   which you can test in an isolated dataset.

5. Optionally, at the end of the implementation phase, you can present a
   dataset that can be used for the following tasks:

   - Unit testing with a validation tool like [Dataform](https://dataform.co/). Unit tests are self-contained, which means that the asset is tested in isolation. In this case, the asset is the table in BigQuery. Unit tests can check for null values, can verify that all strings meet length requirements, and can make sure that certain aggregates produce useful results. Unit tests can include any confidence test that makes sure that the table maintains the organization's business rules.
   - Integration testing with downstream consumers.
   - Peer review.

   This workflow lets you test with production data, without affecting the
   downstream consumers.
6. Before you merge the new data into BigQuery, you can
   create another snapshot. This snapshot is useful as another rollback option
   in case the data in the base table has changed.

   The process of merging the changes depends on the process that your
   organization wants to adopt and on what changes are required. For example,
   for a change in the SQL scripts, the new dataset might be accompanied by a
   pull request to the standard codebase. If the change is limited to a change
   in the data within a given table, you could just copy data using standard
   methods of BigQuery.

You can use a script of stored procedures to encapsulate and automate the steps
for creating a dataset and creating the clones and snapshots. Automating these
tasks reduces risk of human error. For an example of a script that can help
automate the processes, see the
[CI for Data in BigQuery CLI utility](https://github.com/GoogleCloudPlatform/ci-for-data-in-bigquery/)
GitHub repository.

### Benefits of using table clones and table snapshots

When you use the workflow described in the preceding section, your developers
can work in isolation and in parallel, without interfering with their
colleagues. Developers can test and review changes, and if there's an issue,
roll back the changes. Because you're working with table snapshots and not with
full tables, you can minimize costs and storage compared to working with
full tables.

This section provides more detail about how table snapshots and table clones
let developers achieve this workflow. The following diagram shows how table
snapshots and table clones relate to the data in the production dataset.

![A production dataset contains 9 tables. A second dataset named "Dev Dataset 1"
contains snapshots of tables 2 and 3 and clones of tables 2 and 3. A third
dataset named "Dev Dataset 2" contains snapshots of tables 3 and 4 and clones
of tables 3 and 4.](https://docs.cloud.google.com/static/bigquery/images/continuous-integration-of-data-in-bigquery-table-clones-and-snapshots.svg)

In the diagram, the production dataset contains all the tables that are being
used in production. Every developer can create a dataset for their own
development environment. The diagram shows two developer datasets, which are
labeled **Dev Dataset 1** and **Dev Dataset 2**. By using these developer
datasets, developers can work simultaneously on the same tables without
interfering with one another.

After developers have created a dataset, they can create clones and snapshots
of the tables they are working on. The clones and snapshots represent the data
at a particular point in time. At this point, developers are free to change the
table clones, because changes aren't visible on the base table.

A developer can review the table clones, compare them to the snapshot, and test
them for compatibility with downstream consumers. Other developers are able to
work with other clones and snapshots, without interference, and without creating
too many resource-consuming copies of the base table.

Developers can merge changes into the base table while keeping the snapshot
safe to have as a rollback option, if needed. This process can also be repeated
for different environments, like development, test, and production.

## Alternatives to table clones and table snapshots

There are alternatives to using table clones and table snapshots that let you
achieve a similar result. These alternative methods are typically used
differently than clones and snapshots. It's important to understand the
differences between these methods and where you might prefer one method over the
other.

### Copy entire tables into a different dataset

One alternative method is to use a different dataset and to copy the tables
into that dataset. Using this method is similar to using table clones and
snapshots, but you copy the entire set of tables. Depending on the sizes of the
data being copied, the storage costs might be high. Some organizations used this
method before table clones became available in BigQuery. However,
this method doesn't present any advantages over using clones and snapshots.

### Export and import tables to Cloud Storage

Another alternative method is to move the data through Cloud Storage.
This method is also similar to using table clones and table snapshots. However,
it includes the extra step of exporting the data to a
Cloud Storage bucket. One advantage of this method is that it gives you
an extra backup of your data. You might choose this method if you want a backup
for disaster recovery or hybrid solutions.

### Use BigQuery sharing

[BigQuery sharing (formerly Analytics Hub)](https://docs.cloud.google.com/bigquery/docs/analytics-hub-introduction)
lets you share datasets both outside and inside the organization in a way that's
designed to be secure. It offers many features that let you publish datasets to
provide subscribers with controlled, read-only access to those datasets.
However, even though you can use BigQuery sharing to expose datasets
in order to implement changes, a developer still must create table clones in
order to work with the tables.

## Summary of DWH continuous integration options

The following table summarizes the differences, advantages, and potential
disadvantages between the options for DWH continuous integration.
(Sharing offers a different feature set, and is therefore
not measurable using the parameters listed in the table.)

|   | Costs | Rollbacks | Risks |
|---|---|---|---|
| Table snapshots and table clones | Minimal. You pay only for the difference between the snapshot or clone and the base table. | The snapshot acts as a backup to roll back to if necessary. | You control the amount of risk. Snapshots can be taken at a point in time for all tables, which reduces inconsistencies even if there is a rollback. |
| Table copy | Higher costs than using table snapshots and table clones. The entirety of the data is duplicated. To support rollbacks, you need multiple copies of the same table. | Possible, but requires two copies of the table---one copy to serve as backup and one copy to work with and make changes to. | Cloning is harder to do for a point in time. If a rollback is necessary, not all tables are taken from the same point in time. |
| Export and import | Higher costs than using table snapshots and table clones. The data is duplicated. To support rollback, you need multiple copies of the same table. | The exported data serves as a backup. | Exported data is not a point-in-time export for multiple tables. |

## What's next

- Read about BigQuery table snapshots in [Introduction to table snapshots](https://docs.cloud.google.com/bigquery/docs/table-snapshots-intro).
- Learn more about continuous integration for software development in [DevOps tech: Continuous integration](https://docs.cloud.google.com/architecture/devops/devops-tech-continuous-integration).
- For more reference architectures, diagrams, and best practices, explore the [Cloud Architecture Center](https://docs.cloud.google.com/architecture).