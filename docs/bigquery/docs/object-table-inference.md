# Run inference on image object tables

> [!NOTE]
> **Note:** This feature may not be available when using reservations that are created with certain BigQuery editions. For more information about which features are enabled in each edition, see [Introduction to
> BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

This document describes how to use BigQuery ML to run inference
on image [object tables](https://docs.cloud.google.com/bigquery/docs/object-table-introduction).

You can run inference on image data by using an object table as input to the
[`ML.PREDICT` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-predict).

To do this, you must first choose an appropriate model, upload it to
Cloud Storage, and import it into BigQuery by running the
[`CREATE MODEL` statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create).
You can either create your own model, or download one
from [TensorFlow Hub](https://tfhub.dev/).

## Limitations

- Using BigQuery ML [imported models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/inference-overview#inference_using_imported_models) with object tables is only supported when you use capacity-based pricing through reservations; on-demand pricing isn't supported.
- The image files associated with the object table must meet the following requirements:
  - Are less than 20 MB in size.
  - Have a format of JPEG, PNG or BMP.
- The combined size of the image files associated with the object table must be less than 1 TB.
- The model must be one of the following:

  - A [TensorFlow](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow) or [TensorFlow Lite](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tflite) model in [SavedModel](https://www.tensorflow.org/guide/saved_model) format.
  - A PyTorch model in [ONNX format](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-onnx).
- The model must meet the input requirements and limitations described in the
  [`CREATE MODEL` statement for importing TensorFlow models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-tensorflow).

- The serialized size of the model must be less than 450 MB.

- The deserialized (in-memory) size of the model must be less than
  1000 MB.

- The model input tensor must meet the following criteria:

  - Have a data type of `tf.float32` with values in `[0, 1)` or have a data type of `tf.uint8` with values in `[0, 255)`.
  - Have the shape `[batch_size, width, height, 3]`, where:
    - `batch_size` must be `-1`, `None`, or `1`.
    - `width` and `height` must be greater than 0.
- The model must be trained with images in one of the following color spaces:

  - `RGB`
  - `HSV`
  - `YIQ`
  - `YUV`
  - `GRAYSCALE`

  You can use the
  [`ML.CONVERT_COLOR_SPACE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-color-space)
  to convert input images to the color space that the model was trained with.

## Example models

The following models on TensorFlow Hub work with
BigQuery ML and image object tables:

- [ResNet 50](https://tfhub.dev/tensorflow/resnet_50/classification/1). To try using this model, see [Tutorial: Run inference on an object table by using a classification model](https://docs.cloud.google.com/bigquery/docs/inference-tutorial-resnet).
- [MobileNet V3](https://tfhub.dev/google/imagenet/mobilenet_v3_small_075_224/feature_vector/5). To try using this model, see [Tutorial: Run inference on an object table by using a feature vector model](https://docs.cloud.google.com/bigquery/docs/inference-tutorial-mobilenet).

## Required permissions

- To upload the model to Cloud Storage, you need the `storage.objects.create` and `storage.objects.get` permissions.
- To load the model into BigQuery ML, you need the following
  permissions:

  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
- To run inference, you need the following permissions:

  - `bigquery.tables.getData` on the object table
  - `bigquery.models.getData` on the model
  - `bigquery.jobs.create`

## Before you begin

<br />

## Upload a model to Cloud Storage

Follow these steps to upload a model:

1. If you have created your own model, save it locally. If you are using a model from TensorFlow Hub, download it to your local machine. If you are using TensorFlow, this should give you a `saved_model.pb` file and a `variables` folder for the model.
2. If necessary, [create a Cloud Storage bucket](https://docs.cloud.google.com/storage/docs/creating-buckets).
3. [Upload](https://docs.cloud.google.com/storage/docs/uploading-objects) the model artifacts to the bucket.

## Load the model into BigQuery ML

Loading a model that works with image object tables is the same as loading a
model that works with structured data. Follow these steps to load a model into
BigQuery ML:

```googlesql
CREATE MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`
OPTIONS(
  model_type = 'MODEL_TYPE',
  model_path = 'BUCKET_PATH');
```

Replace the following:

- `PROJECT_ID`: your project ID.
- `DATASET_ID`: the ID of the dataset to contain the model.
- `MODEL_NAME`: the name of the model.
- `MODEL_TYPE`: use one of the following values:
  - `TENSORFLOW` for a TensorFlow model
  - `ONNX` for a PyTorch model in ONNX format
- `BUCKET_PATH`: the path to the Cloud Storage bucket that contains the model, in the format `[gs://bucket_name/[folder_name/]*]`.

<br />

The following example uses the default project and loads a TensorFlow
model to BigQuery ML as `my_vision_model`, using the
`saved_model.pb` file and `variables` folder from
`gs://my_bucket/my_model_folder`:

```googlesql
CREATE MODEL `my_dataset.my_vision_model`
OPTIONS(
  model_type = 'TENSORFLOW',
  model_path = 'gs://my_bucket/my_model_folder/*');
```

## Inspect the model

You can inspect the uploaded model to see what its input and output fields
are. You need to reference these fields when you run inference on the object
table.

Follow these steps to inspect a model:

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and click **Datasets**.

4. Click the dataset that contains your model.

5. Click the **Models** tab.

6. In the model pane that opens, click the **Schema** tab.

7. Look at the **Labels** section. This identifies the fields that are output
   by the model.

8. Look at the **Features** section. This identifies the fields that must
   be input into the model. You reference them in the `SELECT` statement
   for the `ML.DECODE_IMAGE` function.

For more detailed inspection of a TensorFlow model, for example to
determine the shape of the model input,
[install TensorFlow](https://www.tensorflow.org/install)
and use the
[`saved_model_cli show` command](https://www.tensorflow.org/guide/saved_model#show_command).

## Preprocess images

You must use the
[`ML.DECODE_IMAGE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-decode-image)
to convert image bytes to a multi-dimensional `ARRAY` representation. You can
use `ML.DECODE_IMAGE` output directly in an `ML.PREDICT` function,
or you can write the results from `ML.DECODE_IMAGE` to a table column and
reference that column when you call `ML.PREDICT`.

The following example writes the output of the `ML.DECODE_IMAGE` function to
a table:

```googlesql
CREATE OR REPLACE TABLE mydataset.mytable AS (
  SELECT ML.DECODE_IMAGE(data) AS decoded_image FROM mydataset.object_table
  );
```

Use the following functions to further process images so that they work with
your model:

- The [`ML.CONVERT_COLOR_SPACE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-color-space) converts images with an `RGB` color space to a different color space.
- The [`ML.CONVERT_IMAGE_TYPE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-convert-image-type) converts the pixel values output by the [`ML.DECODE_IMAGE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-decode-image) from floating point numbers to integers with a range of `[0, 255)`.
- The [`ML.RESIZE_IMAGE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-resize-image) resizes images.

You can use these as part of the `ML.PREDICT` function, or run them on a
table column containing image data output by `ML.DECODE_IMAGE`.

## Run inference

Once you have an appropriate model loaded, and optionally preprocessed
the image data, you can run inference on the image data.

To run inference:

```googlesql
SELECT *
FROM ML.PREDICT(
  MODEL `PROJECT_ID.DATASET_ID.MODEL_NAME`,
  (SELECT [other columns from the object table,] IMAGE_DATA AS MODEL_INPUT
  FROM PROJECT_ID.DATASET_ID.TABLE_NAME)
);
```

Replace the following:

- `PROJECT_ID`: the project ID of the project that contains the model and object table.
- `DATASET_ID`: the ID of the dataset that contains the model and object table.
- `MODEL_NAME`: the name of the model.
- `IMAGE_DATA`: the image data, represented either by the output of the `ML.DECODE_IMAGE` function, or by a table column containing image data output by `ML.DECODE_IMAGE` or other image processing functions.
- `MODEL_INPUT`: the name of an input field for the model. You can find this information by [inspecting the model](https://docs.cloud.google.com/bigquery/docs/object-table-inference#inspect_the_model) and looking at the field names in the **Features** section.
- `TABLE_NAME`: the name of the object table.

<br />

### Examples

**Example 1**

The following example uses the `ML.DECODE_IMAGE` function directly in the
`ML.PREDICT` function. It returns the inference results for all images in the
object table, for a model with an input field of `input` and an output
field of `feature`:

```googlesql
SELECT * FROM
ML.PREDICT(
  MODEL `my_dataset.vision_model`,
  (SELECT uri, ML.RESIZE_IMAGE(ML.DECODE_IMAGE(data), 480, 480, FALSE) AS input
  FROM `my_dataset.object_table`)
);
```

**Example 2**

The following example uses the `ML.DECODE_IMAGE` function directly in the
`ML.PREDICT` function, and uses the `ML.CONVERT_COLOR_SPACE` function in the
`ML.PREDICT` function to convert
the image color space from `RBG` to `YIQ`. It also shows how to
use object table fields to filter the objects included in inference.
It returns the inference results for all JPG images in the
object table, for a model with an input field of `input` and an output
field of `feature`:

```googlesql
SELECT * FROM
  ML.PREDICT(
    MODEL `my_dataset.vision_model`,
    (SELECT uri, ML.CONVERT_COLOR_SPACE(ML.RESIZE_IMAGE(ML.DECODE_IMAGE(data), 224, 280, TRUE), 'YIQ') AS input
    FROM `my_dataset.object_table`
    WHERE content_type = 'image/jpeg')
  );
```

**Example 3**

The following example uses results from `ML.DECODE_IMAGE` that have been
written to a table column but not processed any further. It uses
`ML.RESIZE_IMAGE` and `ML.CONVERT_IMAGE_TYPE` in the `ML.PREDICT` function to
process the image data. It returns the inference results for all images in the
decoded images table, for a model with an input field of `input` and an output
field of `feature`.

Create the decoded images table:

```googlesql
CREATE OR REPLACE TABLE `my_dataset.decoded_images`
  AS (SELECT ML.DECODE_IMAGE(data) AS decoded_image
  FROM `my_dataset.object_table`);
```

Run inference on the decoded images table:

```googlesql
SELECT * FROM
ML.PREDICT(
  MODEL`my_dataset.vision_model`,
  (SELECT uri, ML.CONVERT_IMAGE_TYPE(ML.RESIZE_IMAGE(decoded_image, 480, 480, FALSE)) AS input
  FROM `my_dataset.decoded_images`)
);
```

**Example 4**

The following example uses results from `ML.DECODE_IMAGE` that have been
written to a table column and preprocessed using
`ML.RESIZE_IMAGE`. It returns the inference results for all images in the
decoded images table, for a model with an input field of `input` and an output
field of `feature`.

Create the table:

```googlesql
CREATE OR REPLACE TABLE `my_dataset.decoded_images`
  AS (SELECT ML.RESIZE_IMAGE(ML.DECODE_IMAGE(data) 480, 480, FALSE) AS decoded_image
  FROM `my_dataset.object_table`);
```

Run inference on the decoded images table:

```googlesql
SELECT * FROM
ML.PREDICT(
  MODEL `my_dataset.vision_model`,
  (SELECT uri, decoded_image AS input
  FROM `my_dataset.decoded_images`)
);
```

**Example 5**

The following example uses the `ML.DECODE_IMAGE` function directly in the
`ML.PREDICT` function. In this example, the model has an output field of
`embeddings` and two input fields: one that expects an
image, `f_img`, and one that expects a string, `f_txt`. The image
input comes from the object table and the string input comes from a
standard BigQuery table that is joined with the object table
by using the `uri` column.

```googlesql
SELECT * FROM
  ML.PREDICT(
    MODEL `my_dataset.mixed_model`,
    (SELECT uri, ML.RESIZE_IMAGE(ML.DECODE_IMAGE(my_dataset.my_object_table.data), 224, 224, FALSE) AS f_img,
      my_dataset.image_description.description AS f_txt
    FROM `my_dataset.object_table`
    JOIN `my_dataset.image_description`
    ON object_table.uri = image_description.uri)
  );
```

## What's next

- Learn how to [analyze object tables by using remote functions](https://docs.cloud.google.com/bigquery/docs/object-table-remote-function).
- Try [running inference on an object table by using a feature vector model](https://docs.cloud.google.com/bigquery/docs/inference-tutorial-mobilenet).
- Try [running inference on an object table by using a classification model](https://docs.cloud.google.com/bigquery/docs/inference-tutorial-resnet).
- Try [analyzing an object table by using a remote function](https://docs.cloud.google.com/bigquery/docs/remote-function-tutorial).
- Try [annotating an image with the `ML.ANNOTATE_IMAGE` function](https://docs.cloud.google.com/bigquery/docs/annotate-image).