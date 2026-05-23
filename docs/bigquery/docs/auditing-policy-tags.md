# Audit policy tags

This document describes how to use [Cloud Logging](https://docs.cloud.google.com/logging/docs) to audit activities
related to policy tags. For example, you can determine:

- The email address for the principal that grants or removes access on a policy
  tag

- The email address for whom the access was granted or removed

- The policy tag whose access was changed

## Access to logs

For information about the permission you need to view logs, see the
[Cloud Logging access control guide](https://docs.cloud.google.com/logging/docs/access-control).

## Viewing logs for policy tag events

1. Go to the **Logs Explorer** page in the Google Cloud console.

   [Go to Logs Explorer](https://console.cloud.google.com/logs/query)
2. In the resources drop-down list, click **Audited Resource** , click **Audited
   Resources** again, and then click **datacatalog.googleapis.com**. You will see
   recent audit log entries of Data Catalog resources.

3. To view the log entries, select the Data Catalog
   `SetIamPolicy` method.

4. Click the log entry to see details about the call to the `SetIamPolicy`
   method.

5. Click the log entry fields to see details for the `SetIamPolicy` entry.

   - Click `protoPayload`, then click `authenticationInfo` to see the
     `principalEmail` for the entity that set the IAM policy.

   - Click `protoPayload`, click `request`, click `policy`, and then click
     `bindings` to see the bindings, including principals and roles, that were
     changed.

## What's next

Learn about [best practices for policy tags](https://docs.cloud.google.com/bigquery/docs/best-practices-policy-tags).