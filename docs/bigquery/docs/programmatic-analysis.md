# Programmatic analysis tools

This document describes multiple ways for writing and running code to
analyze data managed in BigQuery.

Although SQL is a powerful query language, programming languages such as
Python, Java, or R provide syntaxes and a large array of built-in statistical
functions that data analysts might find more expressive and easier to manipulate
for certain types of data analysis.

Similarly, while spreadsheets are widely used, other programming environments
like notebooks can sometimes provide a more flexible environment for doing
complex data analysis and exploration.

## Colab Enterprise notebooks

You can use [Colab Enterprise notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction)
in BigQuery to complete
analysis and machine learning (ML) workflows by using SQL, Python, and other
common packages and APIs. Notebooks offer improved collaboration and management
with the following options:

- Share notebooks with specific users and groups by using Identity and Access Management (IAM).
- Review the notebook version history.
- Revert to or branch from previous versions of the notebook.

Notebooks are [BigQuery Studio](https://docs.cloud.google.com/bigquery/docs/query-overview#bigquery-studio)
code assets that are powered by [Dataform](https://docs.cloud.google.com/dataform/docs/overview),
although notebooks aren't visible in Dataform.
[Saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction) are also code assets.
All code assets are stored in a default
[region](https://docs.cloud.google.com/bigquery/docs/programmatic-analysis#supported_regions). Updating the default region changes
the region for all code assets that are created after that point.

Notebook capabilities are available only in the Google Cloud console.

Notebooks in BigQuery offer the following benefits:

- [BigQuery DataFrames](https://docs.cloud.google.com/python/docs/reference/bigframes/latest) is integrated into notebooks, no setup required. BigQuery DataFrames is a Python API that you can use to analyze BigQuery data at scale by using the [pandas DataFrame](https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.html) and [scikit-learn](https://scikit-learn.org/stable/modules/classes.html) APIs.
- Assistive code development powered by [Gemini generative AI](https://docs.cloud.google.com/bigquery/docs/write-sql-gemini).
- Auto-completion of SQL statements, the same as in the BigQuery editor.
- The ability to save, share, and manage versions of notebooks.
- The ability to use [matplotlib](https://matplotlib.org/), [seaborn](https://seaborn.pydata.org/), and other popular libraries to visualize data at any point in your workflow.
- The ability to write and [execute SQL](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) in a cell that can reference Python variables from your notebook.
- Interactive [DataFrame visualization](https://docs.cloud.google.com/bigquery/docs/create-notebooks#cells) that supports aggregation and customization.

You can get started with notebooks by using notebook gallery templates. For more
information, see [Create a notebook using the notebook gallery](https://docs.cloud.google.com/bigquery/docs/create-notebooks#create-notebook-console).

## BigQuery DataFrames

[BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/bigquery-dataframes-introduction)
is a set of open source Python libraries that let you take advantage of
BigQuery data processing by using familiar Python APIs.
BigQuery DataFrames implements the pandas and scikit-learn APIs by
pushing the processing down to BigQuery through SQL conversion.
This design lets you use BigQuery to explore and process terabytes of
data, and also train ML models, all with Python APIs.

BigQuery DataFrames offers the following benefits:

- More than 750 pandas and scikit-learn APIs implemented through transparent SQL conversion to BigQuery and BigQuery ML APIs.
- Deferred execution of queries for enhanced performance.
- Extending data transformations with user-defined Python functions to let you process data in the cloud. These functions are automatically deployed as BigQuery [remote functions](https://docs.cloud.google.com/bigquery/docs/remote-functions).
- Integration with Vertex AI to let you use Gemini models for text generation.

## Other programmatic analysis solutions

The following programmatic analysis solutions are also available in
BigQuery.

### Jupyter notebooks

[Jupyter](https://jupyter.org/) is an open source web-based application for
publishing notebooks that contain live code, textual descriptions, and
visualizations. Data scientists, machine learning specialists, and students
commonly use this platform for tasks such as data cleaning and transformation,
numerical simulation, statistical modeling, data visualization, and ML.

Jupyter Notebooks are built on top of the [IPython](https://ipython.org/)
kernel, a powerful interactive shell, which can interact directly with
BigQuery by using the
[IPython Magics for BigQuery](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/magics).
Alternatively, you can also access BigQuery from your Jupyter
notebooks instances by installing any of the available
[BigQuery clients libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries).
You can visualize
[BigQuery GIS](https://docs.cloud.google.com/bigquery/docs/gis-intro)
data with Jupyter notebooks through
[GeoJSON extension](https://github.com/jupyterlab/jupyter-renderers/tree/master/packages/geojson-extension).
For more details on the BigQuery integration, see the tutorial
[Visualizing BigQuery data in a Jupyter notebook](https://docs.cloud.google.com/bigquery/docs/visualize-jupyter).

![Jupyter notebook chart showing a visualization of BigQuery GIS data.](https://docs.cloud.google.com/static/bigquery/images/dw2bq-reporting-and-analysis-jupyterlab.png)

[JupyterLab](https://jupyterlab.readthedocs.io/en/stable/)
is a web-based user interface for managing documents and activities such as
Jupyter notebooks, text editors, terminals, and custom components. With
JupyterLab, you can arrange multiple documents and activities side by side in
the work area using tabs and splitters.

![JupyterLab: using tabs and splitters to arrange multiple documents and activities side by side in the work area.](https://docs.cloud.google.com/static/bigquery/images/dw2bq-reporting-and-analysis-jupyter-notebook.png)

You can deploy Jupyter notebooks and JupyterLab environments on
Google Cloud by using one of the following products:

- [Vertex AI Workbench instances](https://docs.cloud.google.com/vertex-ai/docs/workbench/instances/introduction), a service that offers an integrated JupyterLab environment in which machine learning developers and data scientists can use some of the latest data science and machine learning frameworks. Vertex AI Workbench is integrated with other Google Cloud data products such as BigQuery, making it easy to go from data ingestion to preprocessing and exploration, and eventually model training and deployment. To learn more, see [Introduction to Vertex AI Workbench instances](https://docs.cloud.google.com/vertex-ai/docs/workbench/instances/introduction).
- [Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc), a fast, easy-to-use, fully managed service for running [Apache Spark](https://spark.apache.org/) and [Apache Hadoop](https://hadoop.apache.org/) clusters in a simple, cost-efficient way. You can install Jupyter notebooks and JupyterLab on a Managed Service for Apache Spark cluster by using the [Jupyter optional component](https://docs.cloud.google.com/dataproc/docs/concepts/components/jupyter). The component provides a Python kernel to run [PySpark](https://pypi.org/project/pyspark/) code. By default, Managed Service for Apache Spark automatically configures notebooks to be [saved in Cloud Storage](https://github.com/src-d/jgscm), making the same notebook files accessible to other clusters. When you migrate your existing notebooks to Managed Service for Apache Spark, check that your notebooks' dependencies are covered by the supported [Managed Service for Apache Spark versions](https://docs.cloud.google.com/dataproc/docs/concepts/versioning/dataproc-versions). If you need to install custom software, consider [creating your own Managed Service for Apache Spark image](https://docs.cloud.google.com/dataproc/docs/guides/dataproc-images), writing your own [initialization actions](https://docs.cloud.google.com/dataproc/docs/concepts/configuring-clusters/init-actions), or [specifying custom Python package requirements](https://docs.cloud.google.com/dataproc/docs/tutorials/python-configuration). To get started, see the tutorial on [Installing and running a Jupyter notebook on a Managed Service for Apache Spark cluster](https://docs.cloud.google.com/dataproc/docs/tutorials/jupyter-notebook).

### Apache Zeppelin

[Apache Zeppelin](https://zeppelin.apache.org/)
is an open source project that offers web-based notebooks for data analytics.
You can deploy an instance of Apache Zeppelin on
[Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc)
by installing the
[Zeppelin optional component](https://docs.cloud.google.com/dataproc/docs/concepts/components/zeppelin).
By default, notebooks are saved in Cloud Storage in the
Managed Service for Apache Spark staging bucket, which is specified by the user or
auto-created when the cluster is created. You can change the notebook location
by adding the property `zeppelin:zeppelin.notebook.gcs.dir` when you create the
cluster. For more information about installing and configuring Apache Zeppelin,
see the
[Zeppelin component guide](https://docs.cloud.google.com/dataproc/docs/concepts/components/zeppelin).
For an example, see
[Analyzing BigQuery datasets using BigQuery Interpreter for Apache Zeppelin](https://cloud.google.com/blog/products/gcp/analyzing-bigquery-datasets-using-bigquery-interpreter-for-apache-zeppelin).

![SQL analysis of the table data, shown in Zeppelin.](https://docs.cloud.google.com/static/bigquery/images/dw2bq-reporting-and-analysis-zeppelin.png)

### Apache Hadoop, Apache Spark, and Apache Hive

For part of your data analytics pipeline migration, you might want to migrate
some legacy
[Apache Hadoop](https://hadoop.apache.org/),
[Apache Spark](https://spark.apache.org/),
or [Apache Hive](https://hive.apache.org/)
jobs that need to directly process data from your data warehouse. For example,
you might extract features for your machine learning workloads.

Managed Service for Apache Spark lets you deploy fully managed Hadoop and Spark clusters
in an efficient, cost-effective way. Managed Service for Apache Spark integrates with
open source [BigQuery connectors](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery).
These connectors use the [BigQuery Storage API](https://docs.cloud.google.com/bigquery/docs/reference/storage),
which streams data in parallel directly from BigQuery through
gRPC.

When you migrate your existing Hadoop and Spark workloads to
Managed Service for Apache Spark, you can check that your workloads' dependencies are
covered by the supported
[Managed Service for Apache Spark versions](https://docs.cloud.google.com/dataproc/docs/concepts/versioning/dataproc-versions).
If you need to install custom software, you might consider
[creating your own Managed Service for Apache Spark image](https://docs.cloud.google.com/dataproc/docs/guides/dataproc-images),
writing your own
[initialization actions](https://docs.cloud.google.com/dataproc/docs/concepts/configuring-clusters/init-actions),
or
[specifying custom Python package requirements](https://docs.cloud.google.com/dataproc/docs/tutorials/python-configuration).

To get started, see the
[Managed Service for Apache Spark quickstart guides](https://docs.cloud.google.com/dataproc/docs/quickstarts)
and the
[BigQuery connector code samples](https://docs.cloud.google.com/dataproc/docs/examples/bigquery-example).

### Apache Beam

[Apache Beam](https://beam.apache.org/)
is an open source framework that provides a rich set of windowing and session
analysis primitives as well as an ecosystem of source and sink connectors,
including a
[connector for BigQuery](https://beam.apache.org/documentation/io/built-in/google-bigquery/).
Apache Beam lets you transform and enrich data both in stream (real time) and
batch (historical) modes with equal reliability and expressiveness.

[Dataflow](https://docs.cloud.google.com/dataflow)
is a fully managed service for running Apache Beam jobs at scale.
The Dataflow serverless approach removes operational overhead
with performance, scaling, availability, security, and compliance handled
automatically so you can focus on programming instead of managing server
clusters.

![Execution graph with an expanded composite transform (MakeMapView). The subtransform that creates the side input (CreateDataflowView) is selected, and the side input metrics are shown in the Step tab.](https://docs.cloud.google.com/static/bigquery/images/dw2bq-reporting-and-analysis-apache-beam.png)

You can submit Dataflow jobs in different ways, either through
the
[command-line interface](https://docs.cloud.google.com/dataflow/docs/guides/using-command-line-intf),
the
[Java SDK](https://beam.apache.org/documentation/sdks/java/),
or the
[Python SDK](https://beam.apache.org/documentation/sdks/python/).

If you want to migrate your data queries and pipelines from other frameworks to
Apache Beam and Dataflow, read about the
[Apache Beam programming model](https://docs.cloud.google.com/dataflow/docs/concepts/beam-programming-model)
and browse the official
[Dataflow documentation](https://docs.cloud.google.com/dataflow/docs).

### Other resources

BigQuery offers a large array of
[client libraries](https://docs.cloud.google.com/bigquery/docs/reference/libraries)
in multiple programming languages such as Java, Go, Python, JavaScript, PHP, and
Ruby. Some data analysis frameworks such as
[pandas](https://pandas.pydata.org/)
provide
[plugins](https://pandas-gbq.readthedocs.io/en/latest/)
that interact directly with BigQuery. For some practical
examples, see the
[Visualize BigQuery data in a Jupyter notebook](https://docs.cloud.google.com/bigquery/docs/visualize-jupyter)
tutorial.

Lastly, if you prefer to write programs in a shell environment, you can use the
[bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool).