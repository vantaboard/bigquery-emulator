# Extend differential privacy

This document provides examples of how to extend differential privacy for
BigQuery differential privacy.

BigQuery lets you extend
[differential privacy](https://docs.cloud.google.com/bigquery/docs/differential-privacy)
to multi-cloud data sources and external differential privacy libraries. This
document provides examples of how to apply differential privacy
for multi-cloud data sources like AWS S3 with BigQuery Omni, how to
call an external differential privacy library using a remote function, and how
to perform differential privacy aggregations with
[PipelineDP](https://pipelinedp.io/), a Python library that can run
with Apache Spark and Apache Beam.

> [!NOTE]
> **Note:** In this document, the privacy parameters in the examples are not recommendations. You should work with your privacy or security officer to determine the optimal privacy parameters for your dataset and organization.

For more information about differential privacy, see
[Use differential privacy](https://docs.cloud.google.com/bigquery/docs/differential-privacy).

## Differential privacy with BigQuery Omni

BigQuery differential privacy supports calls to multi-cloud data
sources like AWS S3. The following example queries an external source of data,
`foo.wikidata`, and applies differential privacy. For more information about the
syntax of the differential privacy clause, see [Differential privacy
clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_clause).

```googlesql
SELECT
  WITH
    DIFFERENTIAL_PRIVACY
      OPTIONS (
        epsilon = 1,
        delta = 1e-5,
        privacy_unit_column = foo.wikidata.es_description)
      COUNT(*) AS results
FROM foo.wikidata;
```

This example returns results similar to the following:

```
-- These results will change each time you run the query.
+---+
| results  |
+---+
| 3465     |
+---+
```

For more information about BigQuery Omni
limitations, see [Limitations](https://docs.cloud.google.com/bigquery/docs/omni-introduction#limitations).

## Call external differential privacy libraries with remote functions

You can call external differential privacy libraries using [remote
functions](https://docs.cloud.google.com/bigquery/docs/remote-functions). The
following link uses a remote function to call an external library hosted by
[Tumult Analytics](https://www.tmlt.dev) to use zero-concentrated
differential privacy on a retail sales dataset.

For information about working with Tumult Analytics, see the
[Tumult Analytics launch post](https://www.tmlt.io/resources/gcp-launch-post) {: .external}.

## Differential privacy aggregations with PipelineDP

PipelineDP is a Python library that performs differential privacy
aggregations and
can run with Apache Spark and Apache Beam. BigQuery can run
Apache Spark stored procedures written in Python. For more information about
running Apache Spark stored procedures, see [Work with stored procedures for
Apache Spark](https://docs.cloud.google.com/bigquery/docs/spark-procedures).

The following example performs a differential privacy aggregation using the
PipelineDP library. It uses the
[Chicago Taxi Trips public dataset](https://docs.cloud.google.com/bigquery/public-data) and computes for each
taxi car - the number of trips, and sum and mean of tips for these trips.

### Before you begin

A standard Apache Spark image does not include PipelineDP. You must create a
[Docker](https://www.docker.com/) image that contains all necessary
dependencies before running a PipelineDP stored procedure. This section
describes how to create and push a Docker image to Google Cloud.

Before you begin, ensure you have installed Docker on your local machine and set
up authentication for pushing Docker images to [gcr.io](https://docs.cloud.google.com/artifact-registry).
For more information about pushing Docker images, see
[Push and pull images](https://docs.cloud.google.com/artifact-registry/docs/pushing-and-pulling).

#### Create and push a Docker image

To create and push a Docker image with required dependencies, follow these steps:

1. Create a local folder `DIR`.
2. Download the [Miniconda installer](https://docs.conda.io/en/latest/miniconda.html#linux-installers), with the Python 3.9 version, to `DIR`.
3. Save the following text to the
   [Dockerfile](https://docs.docker.com/engine/reference/builder/#:%7E:text=A%20Dockerfile%20is%20a%20text,can%20use%20in%20a%20Dockerfile%20).


         # Debian 11 is recommended.
         FROM debian:11-slim

         # Suppress interactive prompts
         ENV DEBIAN_FRONTEND=noninteractive

         # (Required) Install utilities required by Spark scripts.
         RUN apt update && apt install -y procps tini libjemalloc2

         # Enable jemalloc2 as default memory allocator
         ENV LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2

         # Install and configure Miniconda3.
         ENV CONDA_HOME=/opt/miniconda3
         ENV PYSPARK_PYTHON=${CONDA_HOME}/bin/python
         ENV PATH=${CONDA_HOME}/bin:${PATH}
         COPY Miniconda3-py39_23.1.0-1-Linux-x86_64.sh .
         RUN bash Miniconda3-py39_23.1.0-1-Linux-x86_64.sh -b -p /opt/miniconda3 \
         && ${CONDA_HOME}/bin/conda config --system --set always_yes True \
         && ${CONDA_HOME}/bin/conda config --system --set auto_update_conda False \
         && ${CONDA_HOME}/bin/conda config --system --prepend channels conda-forge \
         && ${CONDA_HOME}/bin/conda config --system --set channel_priority strict

         # The following packages are installed in the default image, it is
         # strongly recommended to include all of them.
         RUN apt install -y python3
         RUN apt install -y python3-pip
         RUN apt install -y libopenblas-dev
         RUN pip install \
           cython \
           fastavro \
           fastparquet \
           gcsfs \
           google-cloud-bigquery-storage \
           google-cloud-bigquery[pandas] \
           google-cloud-bigtable \
           google-cloud-container \
           google-cloud-datacatalog \
           google-cloud-dataproc \
           google-cloud-datastore \
           google-cloud-language \
           google-cloud-logging \
           google-cloud-monitoring \
           google-cloud-pubsub \
           google-cloud-redis \
           google-cloud-spanner \
           google-cloud-speech \
           google-cloud-storage \
           google-cloud-texttospeech \
           google-cloud-translate \
           google-cloud-vision \
           koalas \
           matplotlib \
           nltk \
           numba \
           numpy \
           orc \
           pandas \
           pyarrow \
           pysal \
           regex \
           requests \
           rtree \
           scikit-image \
           scikit-learn \
           scipy \
           seaborn \
           sqlalchemy \
           sympy \
           tables \
           virtualenv
         RUN pip install --no-input pipeline-dp==0.2.0

         # (Required) Create the 'spark' group/user.
         # The GID and UID must be 1099. Home directory is required.
         RUN groupadd -g 1099 spark
         RUN useradd -u 1099 -g 1099 -d /home/spark -m spark
         USER spark

4. Run the following command.


       IMAGE=gcr.io/PROJECT_ID/DOCKER_IMAGE:0.0.1
       # Build and push the image.
       docker build -t "${IMAGE}"
       docker push "${IMAGE}"

   Replace the following:
   - `PROJECT_ID`: the project in which you want to create the Docker image.
   - `DOCKER_IMAGE`: the Docker image name.

   The image is uploaded.

### Run a PipelineDP stored procedure

1. To create a stored procedure, use the [CREATE
   PROCEDURE](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_procedure)
   statement.

   ```googlesql
   CREATE OR REPLACE
   PROCEDURE
     `PROJECT_ID.DATASET_ID.pipeline_dp_example_spark_proc`()
     WITH CONNECTION `PROJECT_ID.REGION.CONNECTION_ID`
   OPTIONS (
     engine = "SPARK",
     container_image= "gcr.io/PROJECT_ID/DOCKER_IMAGE")
   LANGUAGE PYTHON AS R"""
   from pyspark.sql import SparkSession
   import pipeline_dp

   def compute_dp_metrics(data, spark_context):
   budget_accountant = pipeline_dp.NaiveBudgetAccountant(total_epsilon=10,
                                                         total_delta=1e-6)
   backend = pipeline_dp.SparkRDDBackend(spark_context)

   # Create a DPEngine instance.
   dp_engine = pipeline_dp.DPEngine(budget_accountant, backend)

   params = pipeline_dp.AggregateParams(
       noise_kind=pipeline_dp.NoiseKind.LAPLACE,
       metrics=[
           pipeline_dp.Metrics.COUNT, pipeline_dp.Metrics.SUM,
           pipeline_dp.Metrics.MEAN],
       max_partitions_contributed=1,
       max_contributions_per_partition=1,
       min_value=0,
       # Tips that are larger than 100 will be clipped to 100.
       max_value=100)
   # Specify how to extract privacy_id, partition_key and value from an
   # element of the taxi dataset.
   data_extractors = pipeline_dp.DataExtractors(
       partition_extractor=lambda x: x.taxi_id,
       privacy_id_extractor=lambda x: x.unique_key,
       value_extractor=lambda x: 0 if x.tips is None else x.tips)

   # Run aggregation.
   dp_result = dp_engine.aggregate(data, params, data_extractors)
   budget_accountant.compute_budgets()
   dp_result = backend.map_tuple(dp_result, lambda pk, result: (pk, result.count, result.sum, result.mean))
   return dp_result

   spark = SparkSession.builder.appName("spark-pipeline-dp-demo").getOrCreate()
   spark_context = spark.sparkContext

   # Load data from BigQuery.
   taxi_trips = spark.read.format("bigquery") \
   .option("table", "bigquery-public-data:chicago_taxi_trips.taxi_trips") \
   .load().rdd
   dp_result = compute_dp_metrics(taxi_trips, spark_context).toDF(["pk", "count","sum", "mean"])
   # Saving the data to BigQuery
   dp_result.write.format("bigquery") \
   .option("writeMethod", "direct") \
   .save("DATASET_ID.TABLE_NAME")
   """;
   ```

   Replace the following:
   - `PROJECT_ID`: the project in which you want to create the stored procedure.
   - `DATASET_ID`: the dataset in which you want to create the stored procedure.
   - `REGION`: the region your project is located in.
   - `DOCKER_IMAGE`: the Docker image name.
   - `CONNECTION_ID`: the name of the connection.
   - `TABLE_NAME`: the name of the table.
2. Use the [CALL](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#call)
   statement to call the procedure.

   ```googlesql
   CALL `PROJECT_ID.DATASET_ID.pipeline_dp_example_spark_proc`()
   ```

   Replace the following:
   - `PROJECT_ID`: the project in which you want to create the stored procedure.
   - `DATASET_ID`: the dataset in which you want to create the stored procedure.

## What's next

- Learn how to [use differential privacy](https://docs.cloud.google.com/bigquery/docs/differential-privacy).
- Learn about the [differential privacy clause](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/query-syntax#dp_clause).
- Learn how to use [differentially private aggregate functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aggregate-dp-functions).