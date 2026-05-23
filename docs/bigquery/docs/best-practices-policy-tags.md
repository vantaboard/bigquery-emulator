# Best practices for using policy tags in BigQuery

This page describes best practices for using policy tags in BigQuery.
Use policy tags to define access to your data when you use
[column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro) or
[dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro).

To learn how to set policy tags on a column, see [Set a policy tag on a column](https://docs.cloud.google.com/bigquery/docs/column-level-security#set_policy).

## Build a hierarchy of data classes

Build a hierarchy of data classes that makes sense for your business.

First, consider what kinds of data the organization processes. Usually
there are a small number of data classes managed by an organization. For
example, an organization could have data classes such as:

- PII data
- Financial data
- Customer order history

A single data class can be applied to multiple data columns using a policy tag.
You should leverage this level of abstraction to efficiently manage many columns
with only a few policy tags.

Second, consider if there are groups of people who need different access to
different data classes. For example, one group needs access to business-
sensitive data such as revenues and customer history. Another group needs access
to personally identifiable data (PII) like phone numbers and addresses.

Keep in mind that you can group policy tags together in a tree. Sometimes it is
helpful to create a root policy tag that contains all of the other policy tags.

The following figure shows an example taxonomy. This hierarchy groups all data
types into three top-level policy tags: **High** , **Medium** , and **Low**.

![Data hierarchy.](https://docs.cloud.google.com/static/bigquery/images/data-hierarchy.png)

Each of the top-level policy tags contains leaf policy tags. For example, the
**High** policy tag contains the **Credit card** , **Government ID** , and
**Biometric** policy tags. The **Medium** and **Low** similarly have leaf policy
tags.

This structure has several benefits:

- You can grant access to an entire group of policy tags at once. For example,
  you can grant the Data Catalog Fine-Grained Reader role on
  the **Low** tier.

- You can move policy tags from one tier to another. For example, you can move
  **Address** from the **Low** tier to the **Medium** tier to further restrict its
  access, without needing to reclassify all **Address** columns.

  > [!NOTE]
  > **Note:** You can move a policy tag only through the Data Catalog `PolicyTagManager.UpdatePolicyTag` method.

- With this fine-grained access, you can manage access to many columns by
  controlling only a small number of data classification policy tags.

For more information about policy tags in BigQuery, see:

- [Introduction to column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security-intro)
- [Restricting access with column-level access control](https://docs.cloud.google.com/bigquery/docs/column-level-security)
- [Introduction to dynamic data masking](https://docs.cloud.google.com/bigquery/docs/column-data-masking-intro)
- [Mask column data by user role](https://docs.cloud.google.com/bigquery/docs/column-data-masking)