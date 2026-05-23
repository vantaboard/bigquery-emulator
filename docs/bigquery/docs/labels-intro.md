# Introduction to labels

To help organize your BigQuery resources, you can add labels to
your datasets, tables, reservations, and views. Labels are key-value pairs that you can attach
to a resource. When you create BigQuery resources, labels are
optional.

After labeling your resources, you can search for them based on label values.
For example, you can use labels to group datasets by purpose, environment,
department, and so on.

## What are labels?

A label is a key-value pair that you can assign to Google Cloud BigQuery resources.
They help you organize these resources and manage your costs at scale, with the
granularity you need. You can attach a label to each resource, then filter the
resources based on their labels. Information about labels is forwarded to the billing system that
lets you break down your billed charges by label. With built-in [billing reports](https://docs.cloud.google.com/billing/docs/how-to/reports),
you can filter and group costs by resource labels.
You can also use labels to
query [billing data exports](https://docs.cloud.google.com/billing/docs/how-to/bq-examples).

## Requirements for labels

The labels applied to a resource must meet the following
requirements:

- Each resource can have up to 64 labels.
- Each label must be a key-value pair.
- Keys have a minimum length of 1 character and a maximum length of 63 characters, and cannot be empty. Values can be empty, and have a maximum length of 63 characters.
- Keys and values can contain only lowercase letters, numeric characters, underscores, and dashes. All characters must use UTF-8 encoding, and international characters are allowed. Keys must start with a lowercase letter or international character.
- The key portion of a label must be unique within a single resource. However, you can use the same key with multiple resources.

These limits apply to the key and value for each label, and to the
individual Google Cloud resources that have labels. There
is no limit on how many labels you can apply across all resources
within a project.

## Common uses of labels

Here are some common use cases for labels:

- **Team or cost center labels** : Add labels based on team or
  cost center to distinguish BigQuery resources owned by different
  teams (for example, `team:research` and `team:analytics`). You can use this
  type of label for cost accounting or budgeting.

- **Component labels** : For example, `component:redis`,
  `component:frontend`, `component:ingest`, and `component:dashboard`.

- **Environment or stage labels** : For example,
  `environment:production` and `environment:test`.

- **State labels** : For example, `state:active`,
  `state:readytodelete`, and `state:archive`.

- **Ownership labels** : Used to identify the teams that are
  responsible for operations, for example: `team:shopping-cart`.


> [!NOTE]
> **Note:** Don't include sensitive information in labels, including personally identifiable information, such as an individual's name or title. Labels are not designed to handle sensitive information.

We don't recommend creating large numbers of unique labels, such as
for timestamps or individual values for every API call.
The problem with this approach is that when the values change frequently or with
keys that clutter the catalog, this makes it difficult to effectively filter and
report on resources.

## Labels and tags

Labels can be used as queryable annotations for resources, but can't be used
to set conditions on policies. Tags provide a way to conditionally allow or
deny policies based on whether a resource has a specific tag, by providing fine-grained
control over policies. For more information, see the
[Tags overview](https://docs.cloud.google.com/resource-manager/docs/tags/tags-overview).

## Limitations

- You can't apply BigQuery labels when using the BigQuery Storage Write API to ingest data.

## What's next

- Learn how to [add labels](https://docs.cloud.google.com/bigquery/docs/adding-labels) to BigQuery resources.
- Learn how to [view labels](https://docs.cloud.google.com/bigquery/docs/viewing-labels) on BigQuery resources.
- Learn how to [update labels](https://docs.cloud.google.com/bigquery/docs/updating-labels) on BigQuery resources.
- Learn how to [filter resources using labels](https://docs.cloud.google.com/bigquery/docs/filtering-labels).
- Learn how to [delete labels](https://docs.cloud.google.com/bigquery/docs/deleting-labels) on BigQuery resources.
- Read about [Using labels](https://docs.cloud.google.com/resource-manager/docs/using-labels) in the Resource Manager documentation.