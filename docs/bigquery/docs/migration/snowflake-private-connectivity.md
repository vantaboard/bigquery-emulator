# Configure private connectivity for Snowflake transfers

This guide shows you how to configure private connectivity to create private data
transfers from Snowflake to BigQuery. Private data transfers
let you transfer data from one source to another all within a private
network, and let you lower security risks when transferring data over the
public internet.

The following sections show you the required steps to configure private
connectivity before you can [create a Snowflake
transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer).

Private transfers are supported for Snowflake instances that are hosted
on Amazon Web Services (AWS), Microsoft Azure, and Google Cloud.

## Create a private link to Snowflake

Create a private link that connects your Snowflake account to your cloud provider. For more information, select one of the following options:

### AWS


[Configure AWS
PrivateLink](https://docs.snowflake.com/en/user-guide/admin-security-privatelink)
to connect your Snowflake account to your AWS
account. Your AWS account must contain the [Amazon S3 staging
bucket required for a Snowflake
transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-s3-bucket).

### Azure


[Configure Azure Private
Link](https://docs.snowflake.com/en/user-guide/privatelink-azure) to connect
your Azure Virtual Network (VNet) to the
Snowflake VNet in Azure. Your
Azure account must contain the [Blob staging bucket required
for a Snowflake
transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-azure-container).

### Google Cloud


[Configure Google Cloud Private Service Connect to connect your
Virtual Private Cloud (VPC) network
subnet](https://docs.snowflake.com/en/user-guide/private-service-connect-google)
to your Snowflake account hosted on Google Cloud. Your
Google Cloud must have a [Cloud Storage staging bucket required for a
Snowflake
transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer#preparing-gcs-bucket).

## Set up Cross-Cloud Interconnect or HA VPN

Set up either Cross-Cloud Interconnect or HA VPN from
AWS or Azure. This step is not required for
Google Cloud-hosted Snowflake accounts.

### AWS

A high availability VPN lets you transfer data through an encrypted VPN
tunnel. To use an HA VPN for your private
Snowflake transfer, see [Create HA VPN
connections between Google Cloud and
AWS](https://docs.cloud.google.com/network-connectivity/docs/vpn/tutorials/create-ha-vpn-connections-google-cloud-aws).

A [Cross-Cloud Interconnect](https://docs.cloud.google.com/network-connectivity/docs/interconnect/concepts/cci-overview)
connection creates a dedicated private link between cloud providers and is
suitable for large data transfers with low-latency requirements. To use
Cross-Cloud Interconnect for your private Snowflake
transfer, see [Connect to
AWS](https://docs.cloud.google.com/network-connectivity/docs/interconnect/how-to/cci/aws/connectivity-overview).

### Azure

A high availability VPN lets you transfer data through an encrypted VPN
tunnel. To use an HA VPN for your private
Snowflake transfer, see [Create HA VPN
connections between Google Cloud and
Azure](https://docs.cloud.google.com/network-connectivity/docs/vpn/tutorials/create-ha-vpn-connections-google-cloud-azure).

A [Cross-Cloud Interconnect](https://docs.cloud.google.com/network-connectivity/docs/interconnect/concepts/cci-overview)
connection creates a dedicated private link between cloud providers and is
suitable for large data transfers with low-latency requirements. To use
Cross-Cloud Interconnect for your private Snowflake
transfer, see [Connect to Azure](https://docs.cloud.google.com/network-connectivity/docs/interconnect/how-to/cci/azure/connectivity-overview).

## Create proxy VM

To complete a private connection, a proxy VM is required to complete the
connection between your data sources without your data reaching the public
internet. This step is required for Snowflake instances hosted on
AWS, Azure, or Google Cloud.

To create and configure a proxy VM for a Snowflake private
transfer, do the following:

1. [Create one or more Compute Engine VM instances](https://docs.cloud.google.com/compute/docs/instances/create-start-instance#create-instance-methods) within the consumer VPC network.
2. Download a TCP proxy software, such as HAProxy or Nginx, and configure the following:
   1. Specify a port. For example, `443`.
   2. Forward all incoming TCP traffic to the private hostname and port on the Snowflake instance.
3. Configure the VMs to resolve the Snowflake private hostname through the DNS configured in the consumer VPC network.
4. Set up an internal passthrough load balancer by doing the following:
   1. [Group the proxy VMs into a managed instance groups (MIG)](https://docs.cloud.google.com/compute/docs/instance-groups/creating-groups-of-managed-instances).
   2. [Set up an internal passthrough Network Load Balancer with VM instance group backends](https://docs.cloud.google.com/load-balancing/docs/internal/setting-up-internal).

## Create service attachment

[Use Private Service Connect to create a network attachment and publish the service](https://docs.cloud.google.com/vpc/docs/configure-private-service-connect-producer#publish-service).
This step is required for Snowflake instances hosted on
AWS, Azure, or Google Cloud.

Your service attachment must be in the same region as your
BigQuery dataset.

If your service uses explicit approval (`connection-preference` is set as
`ACCEPT_MANUAL`), then the service account used in your Snowflake
private data transfer must have the following IAM permissions:

- `compute.serviceAttachments.get`
- `compute.serviceAttachments.update`
- `compute.regionOperations.get`

Once you have created the service attachment, note the service attachment URI. You'll need this URI when you create your Snowflake transfer configuration.

## Create endpoint

Create an endpoint in your AWS or Azure account.
This step is not required for
Google Cloud-hosted Snowflake accounts.

### AWS

In AWS, create a VPC endpoint that connects to Amazon S3.
For more information, see [Access an AWS service using an interface VPC
endpoint](https://docs.aws.amazon.com/vpc/latest/privatelink/create-interface-endpoint.html).

### Azure

Configure a Private Endpoint on the Storage Account in
Azure. For more information, see [Use private endpoints for
Azure Storage](https://learn.microsoft.com/en-us/azure/storage/common/storage-private-endpoints).

Storage Transfer Service requires the `*.blob.core.microsoft.net` endpoint. The
`*.dfs.core.microsoft.net` endpoint isn't supported.

Once created, note the endpoint's IP address. You'll need to specify the IP
address when creating your load balancer in the following section.

## Create a network load balancer

Set up a regional internal proxy network load balancer (NLB) with hybrid
connectivity. You can create the load balancer to route traffic to the
Amazon S3 VPC endpoints or Azure Storage private
endpoints that you created in the preceding section. For more information, see
[Set up a regional internal proxy Network Load Balancer with hybrid connectivity](https://docs.cloud.google.com/storage-transfer/docs/create-transfers/agentless/customer-managed-private-network#set-up-a-regional-internal-proxy-network-load-balancer-with-hybrid-connectivity).

## Register your NLB

After creating your network NLB, register it in the Service Directory
in the Storage Transfer Service. For more information, see [Register your NLB with Service Directory](https://docs.cloud.google.com/storage-transfer/docs/create-transfers/agentless/customer-managed-private-network#register-your-nlb-with-service-directory).

Note the link to the service directory. You'll need the self-link to the service
when you create your Snowflake transfer configuration.

## Create a private Snowflake transfer configuration

[Create the Snowflake transfer](https://docs.cloud.google.com/bigquery/docs/migration/snowflake-transfer). When you set up the
transfer configuration, do the following:

### Console

- For **Use Private Network** , select **True**.
- For **PSC Service Attachment** , enter the service attachment URI. For information about finding the service attachment URI, see [View details for a published
  service](https://docs.cloud.google.com/vpc/docs/configure-private-service-connect-producer#attachment-details). The service attachment URI is in the format `projects/PROJECT_ID/regions/REGION/serviceAttachments/SERVICE_ATTACHMENT`.
- For **Private Network Service** , enter [the self-link of the NLB service](https://docs.cloud.google.com/storage-transfer/docs/create-transfers/agentless/customer-managed-private-network#register-your-nlb-with-service-directory). It uses the format `projects/PROJECT_ID/locations/LOCATION/namespaces/NAMESPACE/services/SERVICE_NAME`.

### bq

- For the `use_private_network` parameter, set to `TRUE`.
- For the `service_attachment` parameter, specify the service attachment URI. For information about finding the service attachment URI, see [View details for a published
  service](https://docs.cloud.google.com/vpc/docs/configure-private-service-connect-producer#attachment-details). The service attachment URI is in the format `projects/PROJECT_ID/regions/REGION/serviceAttachments/SERVICE_ATTACHMENT`.
- For the `private_network_service` parameter, provide the [the self-link of the NLB service](https://docs.cloud.google.com/storage-transfer/docs/create-transfers/agentless/customer-managed-private-network#register-your-nlb-with-service-directory). It uses the format `projects/PROJECT_ID/locations/LOCATION/namespaces/NAMESPACE/services/SERVICE_NAME`.