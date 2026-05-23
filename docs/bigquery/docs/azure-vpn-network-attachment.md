# Set up the Azure-Google Cloud VPN network attachment

This document provides high-level guidance on how to establish a VPN connection
between Google Cloud and Microsoft Azure. The document also includes instructions
for creating a network attachment in Google Cloud.

## Before you begin

Ensure you have the following:

- Access to Azure and Google Cloud accounts with appropriate permissions.
- Existing VPCs in both Azure and Google Cloud.

## Set up networking on Google Cloud

The setup on Google Cloud requires creating the VPC network,
the customer gateway, and the VPN connection.

### Create the VPC network

1. In the Google Cloud console, go to the **VPC networks** page.

   [Go to VPC networks](https://console.cloud.google.com/networking/networks/list)
2. Click **Create VPC network**.

3. Provide a name for the network.

4. Configure subnets as necessary.

5. Click **Create**.

For more information, see
[Create and manage VPC networks](https://docs.cloud.google.com/vpc/docs/create-modify-vpc-networks).

### Create the VPN gateway

> [!NOTE]
> **Note:** The following steps describe how to create a [Classic VPN](https://docs.cloud.google.com/network-connectivity/docs/vpn/concepts/overview#classic-vpn). You can create a high-availability (HA) VPN instead if it fits your use case. For more information, see [Create an HA VPN gateway to a peer VPN gateway](https://docs.cloud.google.com/network-connectivity/docs/vpn/how-to/creating-ha-vpn).

1. In the Google Cloud console, go to the **Cloud VPN gateways** page.

   [Go to Cloud VPN gateways](https://console.cloud.google.com/hybrid/vpn?tab=gateways)
2. Click **Create VPN gateway**.

3. Select the **Classic VPN** option button.

4. Provide a VPN gateway name.

5. Select an existing VPC network in which to create the VPN gateway and tunnel.

6. Select the region.

7. For **IP address** , create or choose an existing regional
   [external IP address](https://docs.cloud.google.com/compute/docs/ip-addresses#reservedaddress).

8. Provide a tunnel name.

9. For **Remote peer IP address**, enter the Azure VPN gateway
   public IP address.

10. Specify options for **IKE version** and **IKE pre-shared key**.

11. Specify the routing options as required to direct traffic to the
    Azure IP ranges.

12. Click **Create**.

For more information, see
[Create a gateway and tunnel](https://docs.cloud.google.com/network-connectivity/docs/vpn/how-to/creating-static-vpns#create_a_gateway_and_tunnel).

## Set up networking on Azure

1. Create the virtual network. For detailed instructions, see [Quickstart: Use the Azure portal to create a virtual network](https://learn.microsoft.com/en-us/azure/virtual-network/quick-create-portal) and [Create a virtual network](https://learn.microsoft.com/en-us/azure/vpn-gateway/tutorial-site-to-site-portal#CreatVNet) in the Azure documentation.
2. Create a VPN routed to the virtual network that you created in the [Create the VPC network](https://docs.cloud.google.com/bigquery/docs/azure-vpn-network-attachment#create-vpc-network) section of this document. For detailed instructions, see [Tutorial: Create and manage a VPN gateway using the Azure portal](https://learn.microsoft.com/en-us/azure/vpn-gateway/tutorial-create-gateway-portal) and [Create a VPN gateway](https://learn.microsoft.com/en-us/azure/vpn-gateway/tutorial-site-to-site-portal#VNetGateway) in the Azure documentation.
3. Create a local network gateway with the public IP address of the Google Cloud VPN gateway and the address space of the Google Cloud network. For detailed instructions, see [Create a local network gateway](https://learn.microsoft.com/en-us/azure/vpn-gateway/tutorial-site-to-site-portal#LocalNetworkGateway) in the Azure documentation.
4. Create a site-to-site VPN connection using the local network gateway that you created. For detailed instructions, see [Create VPN connections](https://learn.microsoft.com/en-us/azure/vpn-gateway/tutorial-site-to-site-portal#CreateConnection) in the Azure documentation.

## Create the Google Cloud network attachment

To attach the network to the Private Service Connect, do the following:

1. In the Google Cloud console, go to the **Private Service Connect** page.

   [Go to Private Service Connect](https://console.cloud.google.com/net-services/psc/list)
2. Select the resource that you want to attach to the network.

3. Click **Edit**.

4. In the **Network attachments** tab, select the network that you created in
   the [Create the VPC network](https://docs.cloud.google.com/bigquery/docs/azure-vpn-network-attachment#create-vpc-network) section of this document.

5. Click **Save**.

For more information, see
[Create network attachments](https://docs.cloud.google.com/vpc/docs/create-manage-network-attachments#create-network-attachments).

## Verify the network connectivity

Ensure that the VMs in Google Cloud can reach the VMs in Azure,
and ensure that the VMs in Azure can reach the VMs in Google Cloud.