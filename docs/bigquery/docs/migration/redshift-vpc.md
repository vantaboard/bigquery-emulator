# Migrating Amazon Redshift data with a VPC network

This document explains how to migrate data from Amazon Redshift to
BigQuery by using a VPC.

If you have a private Amazon Redshift instance in AWS, you can migrate that data to
BigQuery by creating a
[virtual private cloud (VPC) network](https://docs.cloud.google.com/vpc/docs/overview) and connecting it
with the Amazon Redshift VPC network. The data migration process works as follows:

1. You create a VPC network in the project you want to use for the transfer. The VPC network can't be a [Shared VPC](https://docs.cloud.google.com/vpc/docs/shared-vpc) network.
2. You set up a [virtual private network (VPN)](https://docs.cloud.google.com/network-connectivity/docs/vpn/concepts/overview) and connect your project VPC network and the Amazon Redshift VPC network.
3. You specify your project VPC network and a reserved IP range when setting up the transfer.
4. The BigQuery Data Transfer Service creates a tenant project and attaches it to the project you are using for the transfer.
5. The BigQuery Data Transfer Service creates a VPC network with one subnet in the tenant project, using the reserved IP range you specified.
6. The BigQuery Data Transfer Service creates [VPC peering](https://docs.cloud.google.com/vpc/docs/vpc-peering) between your project VPC network and the tenant project VPC network.
7. The BigQuery Data Transfer Service migration runs in the tenant project. It triggers an unload operation from Amazon Redshift to a staging area in an Amazon S3 bucket. Unload speed is determined by your cluster configuration.
8. The BigQuery Data Transfer Service migration transfers your data from the Amazon S3 bucket to BigQuery.

> [!CAUTION]
> **Caution:** The communication between BigQuery and Amazon Redshift happens over the VPN between the peered VPC networks. However, the data movement from Amazon S3 to BigQuery happens over the public internet.

If you'd like to transfer data from your Amazon Redshift instance through public IPs,
you can
[migrate your Amazon Redshift data to BigQuery with these instructions](https://docs.cloud.google.com/bigquery/docs/migration/redshift).

## Before you begin

<br />

### Set required permissions

Before creating an Amazon Redshift transfer, follow these steps:

1. Ensure that the person creating the transfer has the following required
   Identity and Access Management (IAM) permissions in BigQuery:

   - `bigquery.transfers.update` permissions to create the transfer
   - `bigquery.datasets.update` permissions on the target dataset

   The `role/bigquery.admin` predefined IAM role
   includes `bigquery.transfers.update` and `bigquery.datasets.update`
   permissions. For more information on IAM roles in
   BigQuery Data Transfer Service, see [Access control](https://docs.cloud.google.com/bigquery/docs/access-control).
2. Consult the documentation for Amazon S3 to ensure you have
   configured any permissions necessary to enable the transfer. At a minimum,
   the Amazon S3 source data must have the AWS managed policy
   [`AmazonS3ReadOnlyAccess`](https://docs.aws.amazon.com/IAM/latest/UserGuide/access_policies_manage.html#attach-managed-policy-console)
   applied to it.

3. Grant the appropriate
   [IAM permissions](https://docs.cloud.google.com/iam/docs/roles-permissions)
   for creating and deleting VPC Network Peering to the individual setting up the
   transfer. The service uses the individual's Google Cloud user credentials to
   create the VPC peering connection.

   - Permissions to create VPC peering: `compute.networks.addPeering`
   - Permissions to delete VPC peering: `compute.networks.removePeering`

   The `roles/project.owner`, `roles/project.editor`, and
   `roles/compute.networkAdmin` predefined
   IAM roles include the `compute.networks.addPeering`
   and `compute.networks.removePeering` permissions by default.

### Create a dataset

[Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets)
to store your data. You do not need to create any tables.

### Grant access to your Amazon Redshift cluster

Add the following IP ranges of your private Amazon Redshift cluster to an allowlist
by [configuring the security group rules](https://docs.aws.amazon.com/vpc/latest/userguide/working-with-security-group-rules.html).
In a later step, you define the private IP range in this VPC network when you
set up the transfer.

### Grant access to your Amazon S3 bucket

You must have an Amazon S3 bucket to use as a staging area to transfer the
Amazon Redshift data to BigQuery. For detailed instructions, see the
[Amazon documentation](https://aws.amazon.com/premiumsupport/knowledge-center/create-access-key/).

1. We recommended that you create a dedicated Amazon IAM user, and grant that
   user only Read access to Amazon Redshift and Read and Write access to Amazon S3.
   To achieve this step, you can apply the following policies:

   ![Amazon Redshift migration Amazon permissions](https://docs.cloud.google.com/static/bigquery/images/redshift-migration-amazon-permissions.png)
2. Create an Amazon [IAM user access key pair](https://docs.aws.amazon.com/general/latest/gr/aws-sec-cred-types.html).

### Configure workload control with a separate migration queue

Optionally, you can [define an Amazon Redshift queue for migration purposes](https://docs.aws.amazon.com/redshift/latest/dg/cm-c-modifying-wlm-configuration.html)
to limit and separate the resources used for migration. You can configure this
migration queue with a maximum concurrency query count. You can then associate a
certain [migration user group](https://docs.aws.amazon.com/redshift/latest/dg/r_CREATE_GROUP.html)
with the queue and use those credentials when setting up the migration to
transfer data to BigQuery. The transfer service only has
access to the migration queue.

### Gather transfer information

Gather the information that you need to set up the migration with
the BigQuery Data Transfer Service:

- Get the VPC and reserved IP range in Amazon Redshift.

<!-- -->

- Follow [these instructions](https://docs.aws.amazon.com/redshift/latest/mgmt/configure-jdbc-connection.html#obtain-jdbc-url) to get the JDBC URL.
- Get the username and password of a user with appropriate permissions to your Amazon Redshift database.
- Follow the instructions at [Grant access to your Amazon S3 bucket](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#grant_access_to_your_amazon_s3_bucket) to get an AWS access key pair.
- Get the URI of the Amazon S3 bucket you want to use for the transfer. We recommend that you set up a [Lifecycle](https://docs.aws.amazon.com/AmazonS3/latest/user-guide/create-lifecycle.html) policy for this bucket to avoid unnecessary charges. The recommended expiration time is 24 hours to allow sufficient time to transfer all data to BigQuery.

### Assess your data

As part of the data transfer, BigQuery Data Transfer Service writes data from
Amazon Redshift to Cloud Storage as CSV files. If these files contain
the ASCII 0 character, they can't be loaded into BigQuery. We
suggest you assess your data to determine if this could be an issue for you. If
it is, you can work around this by exporting your data to Amazon S3 as Parquet
files, and then importing those files by using BigQuery Data Transfer Service.
For more information, see
[Overview of Amazon S3 transfers](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro).

### Set up the VPC network and the VPN

1. Ensure you have permissions to enable VPC peering. For more information, see [Set required permissions](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#set_required_permissions).

2. Follow the [instructions in this guide](https://docs.cloud.google.com/network-connectivity/docs/vpn/tutorials/create-ha-vpn-connections-google-cloud-aws)
   to set up a Google Cloud VPC network, set up a VPN between your
   Google Cloud project's VPC network and the Amazon Redshift VPC network, and enable
   VPC peering.

   > [!CAUTION]
   > **Caution:** The service uses your VPC network's name as the VPC peering connection name, so ensure there aren't any existing VPC peering connections already using that name.

3. Configure Amazon Redshift to allow connection to your VPN. For more information,
   see [Amazon Redshift cluster security groups](https://docs.aws.amazon.com/redshift/latest/mgmt/working-with-security-groups.html).

4. In the Google Cloud console, go to the **VPC networks** page to verify that your
   Google Cloud VPC network exists in your Google Cloud project is connected to
   Amazon Redshift through the VPN.

   [Go to VPC networks](https://console.cloud.google.com/networking/networks/list)

   The console page lists all of your VPC networks.

### Advertise reserved IPs as custom routes

When providing the range of reserved IP addresses in the transfer configuration,
you must first [add the IP range as a custom route to the existing Cloud Router or BGP session advertisement](https://docs.cloud.google.com/network-connectivity/docs/router/how-to/advertising-custom-ip).

## Set up an Amazon Redshift transfer

Use the following instructions to set up an Amazon Redshift transfer:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Click **Data transfers**.

3. Click **Create transfer**.

4. In the **Source type** section, select **Migration: Amazon Redshift**
   from the **Source** list.

5. In the **Transfer config name** section, enter a name for the transfer,
   such as `My migration`, in the **Display name** field. The display name
   can be any value that allows you to easily identify the transfer if
   you need to modify it later.

6. In the **Destination settings** section, choose
   [the dataset you created](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#create_a_dataset) from the **Dataset** list.

7. In the **Data source details** section, do the following:

   1. For **JDBC connection url for Amazon Redshift** , provide the [JDBC URL](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#jdbc_url) to access your Amazon Redshift cluster.
   2. For **Username of your database**, enter the username for the Amazon Redshift database that you want to migrate.
   3. For **Password of your database**, enter the database password.

      > [!NOTE]
      > **Note:** By providing your Amazon credentials you acknowledge that the BigQuery Data Transfer Service is your agent solely for the limited purpose of accessing your data for transfers.

   4. For **Access key ID** and **Secret access key** , enter the access
      key pair you obtained from
      [Grant access to your S3 bucket](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#grant_access_to_your_S3_bucket).

   5. For **Amazon S3 URI** , enter the [URI of the S3 bucket](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#s3_uri) you'll
      use as a staging area.

   6. For **Amazon Redshift Schema**, enter the Amazon Redshift schema you're
      migrating.

   7. For **Table name patterns** , specify a name or a pattern for matching the
      table names in the schema. You can use regular expressions to
      specify the pattern in the form: `<table1Regex>;<table2Regex>`. The
      pattern should follow Java regular expression syntax. For example:

      - `lineitem;ordertb` matches tables that are named `lineitem` and `ordertb`.
      - `.*` matches all tables.

      Leave this field empty to migrate all tables from the specified schema.

      > [!CAUTION]
      > **Caution:** For very large tables, we recommend transferring one table at a time. [BigQuery has a load quota of 15 TB](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#quotas_and_limits) per load job.

   8. For **VPC and the reserved IP range**, specify your VPC network name
      and the private IP address range to use in the tenant project VPC network.
      Specify the IP address range as a CIDR block.

      ![Amazon Redshift migration CIDR field](https://docs.cloud.google.com/static/bigquery/images/redshift-migration-cidr-field.png)
      - The IP addresses must first be advertised as a custom route. For more information, see [Advertise reserved IPs as custom routes](https://docs.cloud.google.com/bigquery/docs/migration/redshift-vpc#advertise-routes).
      - The form is `VPC_network_name:CIDR`, for example: `my_vpc:10.251.1.0/24`.
      - Use standard private VPC network address ranges in the CIDR notation, starting with `10.x.x.x`.
      - The IP range must have more than 10 IP addresses.
      - The IP range must not overlap with any subnet in your project VPC network or the Amazon Redshift VPC network.
      - If you have multiple transfers configured for the same Amazon Redshift instance, make sure to use the same `VPC_network_name:CIDR` value in each, so that multiple transfers can reuse the same migration infrastructure.

      > [!CAUTION]
      > **Caution:** After being configured, the value of this CIDR block is immutable.

8. Optional: In the **Notification options** section, do the following:

   1. Click the toggle to enable email notifications. When you enable this option, the transfer administrator receives an email notification when a transfer run fails.
   2. For **Select a Pub/Sub topic** , choose your [topic](https://docs.cloud.google.com/pubsub/docs/overview#types) name or click **Create a topic** . This option configures Pub/Sub run [notifications](https://docs.cloud.google.com/bigquery/docs/transfer-run-notifications) for your transfer.
9. Click **Save**.

10. The Google Cloud console displays all the transfer setup details,
    including a **Resource name** for this transfer.

## Quotas and limits

Migrating an Amazon Redshift private instance with a VPC network runs the migration
agent on a single tenant infrastructure. Due to computation resource limits,
at most 5 concurrent transfer runs are allowed.

BigQuery has a load quota of 15 TB for each load job for each
table.
Internally, Amazon Redshift compresses the table data, so the exported table size
will be larger than the table size reported by Amazon Redshift. If you plan
to migrate a table larger than 15 TB, please contact
[Cloud Customer Care](https://docs.cloud.google.com/bigquery/docs/getting-support) first.

Costs can be incurred outside of Google by using this service. Review the [Amazon Redshift](https://aws.amazon.com/redshift/pricing/) and
[Amazon S3](https://aws.amazon.com/s3/pricing/) pricing pages for details.

Because of
[Amazon S3's consistency model](https://docs.cloud.google.com/bigquery/docs/s3-transfer-intro#consistency_considerations),
it's possible that some files will not be included in the transfer to
BigQuery.

## What's next

- Learn about standard [Amazon Redshift migrations](https://docs.cloud.google.com/bigquery/docs/migration/redshift).
- Learn more about the [BigQuery Data Transfer Service](https://docs.cloud.google.com/bigquery/docs/dts-introduction).
- Migrate SQL code with the [Batch SQL translation](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator).