# Configure connections with network attachments

BigQuery supports federated queries that let you send a query
statement to external databases and get the result back as a temporary table.
Federated queries use the BigQuery Connection API to establish a connection. This
document shows you how to increase the security of this connection.

Because the connection connects directly to your database, you must allow
traffic from Google Cloud to your database engine. To increase security, you
should only allow traffic that comes from your BigQuery queries.
This traffic restriction can be accomplished in one of two ways:

- By defining a static IP address that is used by a BigQuery connection and adding it to the firewall rules of the external data source.
- By creating a VPN between BigQuery and your internal infrastructure, and using it for your queries.

Both of these techniques are supported through the use of
[network attachments](https://docs.cloud.google.com/vpc/docs/create-manage-network-attachments).

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document.

### Required roles


To get the permissions that
you need to configure a connection with network attachments,

ask your administrator to grant you the
[Compute Admin](https://docs.cloud.google.com/iam/docs/roles-permissions/compute#compute.admin) (`roles/compute.admin`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to configure a connection with network attachments. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to configure a connection with network attachments:

- `compute.networkAttachments.get`
- `compute.networkAttachments.update`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

For more information about IAM roles and permissions in
BigQuery, see
[BigQuery IAM roles and permissions](https://docs.cloud.google.com/bigquery/docs/access-control).

## Limitations

Connections with network attachments are subject to the following limitations:

- Network attachments are supported only for [SAP Datasphere connections](https://docs.cloud.google.com/bigquery/docs/sap-datasphere-federated-queries).
- For standard regions, network attachments must be located in the same region as the connection. For connections in the `US` multi-region, the network attachment must be located in the `us-central1` region. For connections in the `EU` multi-region, the network attachment must be located in the `europe-west4` region.
- You can't make any changes to your network attachment after you create it. To configure anything in a new way, you need to recreate the network attachment.
- Network attachments can't be deleted unless the producer (BigQuery) deletes the allocated resources. To initiate the deletion process, you must [contact BigQuery support](https://docs.cloud.google.com/bigquery/docs/support).

## Create a network attachment

When you create a connection for query federation, you can use the optional
network attachment parameter, which points to a network attachment that provides
connectivity to the network from which the connection to your database is
established. You can create a network attachment by either defining a static IP
address or creating a VPN. For either option, do the following:

1. If you don't already have one,
   [create a VPC network and subnet](https://docs.cloud.google.com/vpc/docs/create-modify-vpc-networks#create-custom-network).

2. If you want to create a network attachment by defining a static IP address,
   [create a Cloud NAT gateway with a static IP address](https://docs.cloud.google.com/nat/docs/set-up-manage-network-address-translation#create-nat-gateway),
   using the network, region, and subnet that you created. If you want to
   create a network attachment by creating a VPN, create a
   [VPN that is connected to your private network](https://docs.cloud.google.com/network-connectivity/docs/vpn).

3. [Create a network attachment](https://docs.cloud.google.com/vpc/docs/create-manage-network-attachments#create-manual-accept)
   using the network, region, and subnet that you created.

4. Optional: Depending on your organization's security policies, you might need
   to configure your Google Cloud firewall to allow egress by
   [creating a firewall rule](https://docs.cloud.google.com/firewall/docs/using-firewalls#creating_firewall_rules)
   with the following settings:

   - Set **Targets** to **All instances in the network**.
   - Set **Destination IPv4 ranges** to the entire IP address range.
   - Set **Specified protocols and ports** to the port that is used by your database.
5. Configure your internal firewall to allow ingress from the static IP address
   that you created. This process varies by data source.

6. [Create a connection](https://docs.cloud.google.com/bigquery/docs/connections-api-intro), and include the
   name of the network attachment that you created.

7. Run any [federated query](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro) to
   synchronize your project with the network attachment.

Your connection is now configured with a network attachment, and you can run
federated queries.

## Pricing

- Standard [federated query pricing](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#pricing) applies.
- Using VPC is subject to [Virtual Private Cloud pricing](https://cloud.google.com/vpc/pricing).
- Using Cloud VPN is subject to [Cloud VPN pricing](https://docs.cloud.google.com/network-connectivity/docs/vpn/pricing).
- Using Cloud NAT is subject to [Cloud NAT pricing](https://cloud.google.com/nat/pricing).

## What's next

- Learn about different [connection types](https://docs.cloud.google.com/bigquery/docs/connections-api-intro).
- Learn about [managing connections](https://docs.cloud.google.com/bigquery/docs/working-with-connections).
- Learn about [federated queries](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro).