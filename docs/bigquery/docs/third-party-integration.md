# Integrate with third-party tools

This document describes initial configuration steps you might need to take to
manage the connection between BigQuery and your third-party
business intelligence (BI) solutions. If you need assistance with a solution,
consider contacting a [Google Cloud Ready - BigQuery
partner](https://console.cloud.google.com/bigquery/partner-center). Third-party software is not
supported by Cloud Customer Care when it has been identified that
BigQuery is working as intended.

### Network connectivity

All BI and data analytics solutions that are deployed on hosts and services
with external IP addresses can access BigQuery through the public
[BigQuery REST API](https://docs.cloud.google.com/bigquery/docs/reference/rest)
and the RPC-based
[BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage)
over the internet.

Third-party BI and data analytics solutions that are deployed on
[Compute Engine](https://docs.cloud.google.com/compute)
VM instances only with internal IP addresses (no external IP addresses) can use
[Private Google Access](https://docs.cloud.google.com/vpc/docs/private-google-access)
to reach Google APIs and services like BigQuery. You enable
Private Google Access on a subnet-by-subnet basis; it's a setting for subnets
in a VPC network. To enable a subnet for Private Google Access and to view the
requirements, see
[Configuring Private Google Access](https://docs.cloud.google.com/vpc/docs/configure-private-google-access).

Third-party BI and data analytics solutions that are deployed on on-premises
hosts can use
[Private Google Access for on-premises hosts](https://docs.cloud.google.com/vpc/docs/private-google-access-hybrid)
to reach Google APIs and services like BigQuery. This service
establishes a private connection over a
[Cloud VPN](https://docs.cloud.google.com/network-connectivity/docs/vpn)
or
[Cloud Interconnect](https://docs.cloud.google.com/network-connectivity/docs/interconnect)
from your data center to Google Cloud. On-premises hosts don't
need external IP addresses; instead, they use internal
[RFC 1918](https://tools.ietf.org/html/rfc1918)
IP addresses. To enable Private Google Access for on-premises hosts, you
must configure DNS, firewall rules, and routes in your on-premises and VPC
networks. For more details on Private Google Access for on-premises
hosts, see
[Configuring Private Google Access for on-premises hosts](https://docs.cloud.google.com/vpc/docs/configure-private-google-access-hybrid).

If you opt to manage your own instance of a third-party BI solution, consider
deploying it on
[Compute Engine](https://docs.cloud.google.com/compute)
to take advantage of Google's network backbone and minimize latency between your
instance and BigQuery.

If your BI solution supports it, you might consider setting
filters in report or dashboard queries whenever possible.
This step pushes the filters as
`WHERE` clauses to BigQuery. Although setting these filters
doesn't reduce the amount of data that BigQuery scans, it does
reduce the amount of data that comes back over the network.

For more information on network and query optimizations, see
[Migrating data warehouses to BigQuery: performance optimization](https://docs.cloud.google.com/architecture/dw2bq/dw-bq-performance-optimization)
and the
[Introduction to optimizing query performance](https://docs.cloud.google.com/bigquery/docs/best-practices-performance-overview).

### API and ODBC/JDBC integrations

Google's BI and data analytics products like
[Data Studio](https://docs.cloud.google.com/looker-studio),
[Looker](https://docs.cloud.google.com/looker),
[Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc),
and
[Vertex AI Workbench instances](https://docs.cloud.google.com/vertex-ai/docs/workbench/instances/introduction),
and third-party solutions like
[Tableau](https://www.tableau.com/),
offer direct BigQuery integration using the
[BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/libraries-overview).

For other third-party solutions and custom applications, Google has collaborated with
[Magnitude Simba](https://www.magnitude.com/products/data-connectivity)
to provide
[ODBC](https://wikipedia.org/wiki/Open_Database_Connectivity)
and
[JDBC](https://wikipedia.org/wiki/Java_Database_Connectivity)
drivers.
The intent of these drivers is to help you leverage the power of
BigQuery with existing tooling and infrastructure that doesn't
integrate with the [BigQuery API](https://docs.cloud.google.com/bigquery/docs/reference/libraries-overview).

For more details, see the Google Cloud documentation on
[ODBC and JDBC Drivers for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/odbc-jdbc-drivers).

### Authentication

The BigQuery API uses
[OAuth 2.0](https://tools.ietf.org/html/rfc6749)
access tokens to authenticate requests. An OAuth 2.0 access token is a string
that grants temporary access to an API.
[Google's OAuth 2.0 server](https://developers.google.com/identity/protocols/OAuth2WebServer#obtainingaccesstokens)
grants access tokens for all Google APIs. Access tokens are associated with a
[scope](https://tools.ietf.org/html/rfc6749#section-3.3),
which limits the token's access. For scopes associated with the
BigQuery API, see the complete
[list of Google API scopes](https://developers.google.com/identity/protocols/googlescopes#bigqueryv2).

BI and data analytics solutions that offer native BigQuery
integration can automatically generate access tokens for BigQuery
either by using
[OAuth 2.0 protocols](https://developers.google.com/identity/protocols/OAuth2)
or customer-supplied
[service account private keys](https://docs.cloud.google.com/iam/docs/creating-managing-service-account-keys).
Similarly, solutions that rely on Simba ODBC/JDBC drivers can also obtain access
tokens
[for a Google user account](https://www.simba.com/products/BigQuery/doc/ODBC_InstallGuide/mac/content/odbc/bq/configuring/authenticating/useraccount.htm)
or
[for a Google service account](https://www.simba.com/products/BigQuery/doc/ODBC_InstallGuide/mac/content/odbc/bq/configuring/authenticating/serviceaccount.htm).