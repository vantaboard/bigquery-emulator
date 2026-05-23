# The ML.PRINCIPAL_COMPONENT_INFO function

This document describes the `ML.PRINCIPAL_COMPONENT_INFO` function, which lets
you see the statistics of the principal components in a principal component
analysis (PCA) model, such as
[eigenvalue](https://en.wikipedia.org/wiki/Eigenvalues_and_eigenvectors)
and explained variance ratio.

## Syntax

```sql
ML.PRINCIPAL_COMPONENT_INFO(
  MODEL `PROJECT_ID.DATASET.MODEL`
)
```

### Arguments

`ML.PRINCIPAL_COMPONENT_INFO` takes the following arguments:

- `PROJECT_ID`: your project ID.
- `DATASET`: the BigQuery dataset that contains the model.
- `MODEL`: the name of the model.

## Output

`ML.PRINCIPAL_COMPONENT_INFO` returns the following columns:

- `principal_component_id`: an `INT64` that contains the principal component. The table is ordered in descending order of the `eigenvalue` value.
- `eigenvalue`: a `FLOAT64` value that contains the factor by which the eigenvector is scaled. Eigenvalue and explained variance are the same concepts in PCA.
- `explained_variance_ratio`: a `FLOAT64` value that contains the explained variance ratio, which is the ratio between the variance, also known as eigenvalue, of that principal component and the total variance. The total variance is the sum of the variances from all of the individual principal components.
- `cumulative_explained_variance_ratio`: a `FLOAT64` value that contains the cumulative explained variance ratio of the k-th principal component, which is the sum of the explained variance ratios of all the previous principal components, including the k-th principal component.

## Example

The following example retrieves the eigenvalue-related information of each
principal component in the model `mydataset.mymodel` in your default project.

```sql
SELECT
  *
FROM
  ML.PRINCIPAL_COMPONENT_INFO(MODEL `mydataset.mymodel`)
```

## What's next

- For more information about model weights support in BigQuery ML, see [BigQuery ML model weights overview](https://docs.cloud.google.com/bigquery/docs/weights-overview).
- For more information about supported SQL statements and functions for ML models, see [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey).