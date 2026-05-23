# The ML.RESIZE_IMAGE function

This document describes the `ML.RESIZE_IMAGE` scalar function, which lets you
resize images by using
[bilinear interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation).
You can use `ML.RESIZE_IMAGE` with the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict)
or chain it with other functions or subqueries.

## Syntax

```sql
ML.RESIZE_IMAGE(image, target_height, target_width, preserve_aspect_ratio)
```

### Arguments

`ML.RESIZE_IMAGE` takes the following arguments:

- `image`: a `STRUCT` value that represents an image, in one of the following
  forms:

  - `STRUCT<ARRAY<INT64>, ARRAY<FLOAT64>>`
  - `STRUCT<ARRAY<INT64>, ARRAY<INT64>>`

  The first array in the struct must contain the dimensions of the image.
  It must contain three `INT64` values, which represent the image height (H),
  width (W), and number of channels (C).

  The second array in the struct must contain the image data. The
  length of the array must be equivalent to H x W x C from the preceding
  array. If the image data is in `FLOAT64`, each value in the array must be
  between `[0, 1)`. If the image data is in `INT64`, each value in the array
  must be between `[0, 255)`.

  For example, a 1x2 image in the `RGB` color space might have a form
  similar to `[[0.1,0.2,0.3],[0.4,0.5,0.6]]`, which would be represented
  as a struct in the form `([1,2,3],[0.1,0.2,0.3,0.4,0.5,0.6])`.

  The struct value must be \<= 60 MB.
- `target_height`: an `INT64` value that specifies the height of the resized
  image in pixels, or the maximum acceptable height if `preserve_aspect_ratio`
  is set to `TRUE`.

- `target_width`: an `INT64` value that specifies the width of the resized
  image in pixels, or the maximum acceptable width if `preserve_aspect_ratio`
  is set to `TRUE`.

- `preserve_aspect_ratio`: a `BOOL` value that indicates whether
  the existing height and width ratio of the image is preserved in the
  resized image. If set to `TRUE`, the function returns the largest possible
  image that falls within the specified height and width constraints but
  still maintains the aspect ratio.

## Output

`ML.RESIZE_IMAGE` returns a `STRUCT` value that represents the resized image,
with the same form as the `image` input argument.

The first array in the struct represents the dimensions of the image, and
the second array in the struct contains the image data, similar
to the `image` input argument.

> [!NOTE]
> **Note:** If you reference `ML.RESIZE_IMAGE` in SQL statements in the BigQuery editor, it is possible for the function output to be too large to display. If this occurs, write the output to a table instead.

## Examples

**Example 1**

The following example resizes input images to have a height and width
of 240 pixels:

```sql
CREATE OR REPLACE TABLE `mydataset.results`
AS (
  SELECT uri, prediction_results
  FROM
    ML.PREDICT(
      MODEL `mydataset.mymodel`,
      SELECT
        ML.RESIZE_IMAGE(ML.DECODE_IMAGE(data), 240, 240, FALSE)
        AS image,
        uri
      FROM `mydataset.images`)
);
```

**Example 2**

The following example resizes input images while preserving the aspect ratio.
With the settings shown, the function returns an image with dimensions of
`(10, 100)` for an input image with dimensions of `(20, 200)`.

```sql
CREATE OR REPLACE TABLE `mydataset.results`
AS (
  SELECT uri, prediction_results
  FROM
    ML.PREDICT(
      MODEL `mydataset.mymodel`,
      SELECT
        ML.RESIZE_IMAGE(ML.DECODE_IMAGE(data), 10, 120, TRUE)
        AS image,
        uri
      FROM `mydataset.images`)
);
```

## What's next

- For more information about feature preprocessing, see [Feature preprocessing overview](https://docs.cloud.google.com/bigquery/docs/preprocess-overview).
- For more information about supported SQL statements and functions for each
  model type, see the following documents:

  - [End-to-end user journeys for generative AI models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-genai)
  - [End-to-end user journeys for time series forecasting models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-forecast)
  - [End-to-end user journeys for ML models](https://docs.cloud.google.com/bigquery/docs/e2e-journey)
  - [End-to-end user journeys for imported models](https://docs.cloud.google.com/bigquery/docs/e2e-journey-import)
  - [Contribution analysis user journey](https://docs.cloud.google.com/bigquery/docs/contribution-analysis#contribution_analysis_user_journey)