# Configure Cloud SQL instance access

This document provides detailed steps for setting up Virtual Private Cloud peering,
installing a Cloud SQL proxy, and connecting to an internal
Cloud SQL IP address across different Google Cloud projects.
This setup ensures security-enhanced and efficient communication between your
Cloud SQL instance and the following connectors:

- [The BigQuery Data Transfer Service MySQL connector](https://docs.cloud.google.com/bigquery/docs/mysql-transfer)
- [The BigQuery Data Transfer Service PostgreSQL connector](https://docs.cloud.google.com/bigquery/docs/postgresql-transfer).

This document also covers the creation of a network attachment in the
BigQuery Data Transfer Service connector project.

## Before you begin

Ensure you have the following:

- Access to a Google Cloud project with the BigQuery Data Transfer Service connector and another Google Cloud project with the Cloud SQL instance.
- An existing MySQL or PostgreSQL database in a Google Cloud project.
- The appropriate permissions to [create a VPC](https://docs.cloud.google.com/vpc/docs/create-modify-vpc-networks), [create firewall rules](https://docs.cloud.google.com/network-connectivity/docs/vpn/how-to/configuring-firewall-rules), and install software.
- A [virtual machine (VM) instance](https://docs.cloud.google.com/compute/docs/instances/create-start-instance).

## Set up VPC peering

To set up VPC peering, you must create VPC
peering from the BigQuery Data Transfer Service connector
project, create VPC peering in the Cloud SQL database
project to the BigQuery Data Transfer Service project, and configure the routes and
firewall rules.

### Create VPC peering from the BigQuery Data Transfer Service connector project

1. In the Google Cloud console, go to the **VPC network peering** page for your
   BigQuery Data Transfer Service connector project.

   [Go to VPC Network Peering](https://console.cloud.google.com/networking/peering/list)
2. Click **Create peering connection**.

3. In the **Name** field, enter a name for your peering configuration.

4. For **Your VPC network**, select the VPC network that you want
   to peer in the BigQuery Data Transfer Service connector project.

5. For **Peered VPC network** , select the **In another project** option.

6. For **Project ID**, enter the project ID of the Cloud SQL
   project.

7. For **VPC network name**, enter the name of the VPC network
   in the Cloud SQL project.

8. Click **Create**.

### Create VPC peering in the Cloud SQL database project

To create VPC peering in the Cloud SQL database project
to the BigQuery Data Transfer Service project, do the following:

1. In the Google Cloud console, go to the **VPC Network Peering** page for your
   BigQuery Data Transfer Service connector project.

   [Go to VPC Network Peering](https://console.cloud.google.com/networking/peering/list)
2. Click **Create peering connection**.

3. In the **Name** field, enter a name for your peering configuration.

4. Select the VPC network that you want to peer in the
   Cloud SQL database project.

5. For **Peer project ID**, enter the project ID of the
   BigQuery Data Transfer Service project.

6. For **Peered VPC network**, enter the name of the VPC network
   in the BigQuery Data Transfer Service connector project.

7. Click **Create**.

### Configure routes and firewall rules

If you didn't select import-export routes while configuring the peering
connections earlier, follow these steps to do so now:

1. Go to the **Routes** page for your BigQuery Data Transfer Service
   connector project.

   [Go to Routes](https://console.cloud.google.com/networking/routes/list)
2. Ensure the routes exist to allow traffic between the peered VPC
   environments.

3. Go to the **Firewall policies** page.

   [Go to Firewall policies](https://console.cloud.google.com/networking/firewalls/list)
4. Create firewall rules to allow for traffic on the necessary ports (for
   example, port 3306 for MySQL and port 5432 for PostgreSQL)
   between the peered networks.

5. Add the custom firewall rules that are required from the BigQuery Data Transfer Service
   connector project to the Cloud SQL database-hosted
   project.

6. Configure routes and firewall rules for your project with the
   Cloud SQL instance as you did in the previous steps.

## Set up the Cloud SQL proxy

1. Use SSH to connect to a virtual machine (VM) instance in the
   BigQuery Data Transfer Service connector project.

2. In the terminal, download the Cloud SQL proxy:

       wget https://dl.google.com/cloudsql/cloud_sql_proxy.linux.amd64 -O cloud_sql_proxy

3. Update the permissions for the downloaded files:

       chmod +x cloud_sql_proxy

4. Run the Cloud SQL proxy:

       ./cloud_sql_proxy -instances=NAME=tcp:3306 or 5432 &

   Replace `NAME` with the name of your
   Cloud SQL instance connection.

## Connect to the internal Cloud SQL IP address

1. Use the internal IP address of the Cloud SQL instance for connections.
2. Configure your application or tool to connect to the internal IP address, specifying the appropriate credentials and database details.

When connecting from a different Google Cloud project, use the internal
IP address of the proxy VM that you deployed earlier. This solution resolves
transitive peering issues.

## Create the network attachment

To create the network attachment in the BigQuery Data Transfer Service connector
project, follow these steps:

1. In the Google Cloud console, go to the **Network attachments** page.

   [Go to Network attachments](https://console.cloud.google.com/net-services/psc/list/networkAttachments)
2. Click **Create network attachment**.

3. Provide a name for the network attachment.

4. Select the appropriate VPC network.

5. For **Region**, specify the region where your BigQuery Data Transfer Service connector is located.

6. For **Subnetwork**, select the appropriate option that matches your setup.

7. Click **Create network attachment**.

## Test the connection

1. Verify that the VM with the Cloud SQL proxy can connect to the
   Cloud SQL instance:

       mysql -u USERNAME -p -h IP_ADDRESS

   Replace the following:
   - `USERNAME`: the username of the database user
   - `IP_ADDRESS`: the IP address of the Cloud SQL instance
2. Ensure that applications in the BigQuery Data Transfer Service connector
   project can connect to the Cloud SQL instance using the internal IP.

## Troubleshoot

If you are having issues setting up your network configuration, do the following:

- Ensure that VPC peering is established and that routes are correctly configured.
- Verify that the firewall rules allow for traffic on the required ports.
- Check the Cloud SQL proxy logs for errors and ensure it is running correctly.
- Ensure that the network attachment is correctly configured and connected.