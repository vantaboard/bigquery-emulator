# The DROP MODEL statement

To delete a model in BigQuery ML, use the `DROP MODEL` or the `DROP
MODEL IF EXISTS` DDL statement.

The `DROP MODEL` DDL statement deletes a model in the specified dataset. If
the model name does not exist in the dataset, the following error is returned:

`Error: Not found: Model myproject:mydataset.mymodel`

The `DROP MODEL IF EXISTS` DDL statement deletes a model in the specified
dataset only if the model exists. If the model name does not exist in the
dataset, no error is returned, and no action is taken.

If you are deleting a model in another project, you must specify the project,
dataset, and model in the following format: \`\[PROJECT\].\[DATASET\].\[MODEL\]\`
(including the backticks); for example, \`myproject.mydataset.mymodel\`.

For more information about supported SQL statements and functions for different
model types, see the following documents:

- [End-to-end user journeys for generative AI models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-genai)
- [End-to-end user journeys for time series forecasting models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-forecast)
- [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
- [End-to-end user journeys for imported models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-import)

## Syntax

```
{DROP MODEL | DROP MODEL IF EXISTS}
model_name
```

Where:

**`{DROP MODEL | DROP MODEL IF EXISTS}`** is one of the following statements:

- `DROP MODEL` --- deletes a model in the specified dataset
- `DROP MODEL IF EXISTS` --- deletes a model only if the model exists in the specified dataset

**`model_name`** is the name of the model you're deleting.