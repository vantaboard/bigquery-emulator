# BigQuery locations

This page explains the concept of *location* and the different regions
where data can be stored and processed. Pricing for storage and analysis is also
defined by location of data and reservations. For more information about pricing
for locations, see [BigQuery pricing](https://cloud.google.com/bigquery/pricing). To learn
how to set the location for your dataset, see [Create datasets](https://docs.cloud.google.com/bigquery/docs/datasets). For
information about reservation locations, see [Managing reservations in different
regions](https://docs.cloud.google.com/bigquery/docs/reservations-workload-management#manage_reservations_in_different_regions).

For more information about how the BigQuery Data Transfer Service uses location, see
[Data location and transfers](https://docs.cloud.google.com/bigquery/docs/dts-locations).

## Locations and regions

BigQuery provides two types of data and compute locations:

- A *region* is a specific geographic place, such as London.

- A *multi-region* is a large geographic area, such as the United States or
  Europe, that contains many unique and discrete regions. Multi-region locations can
  provide larger quotas than single regions, but multi-regions don't
  provide regional redundancy. Data is
  stored in a single region and compute is only provided within that region. For
  cross-region redundancy BigQuery offers [managed disaster
  recovery](https://docs.cloud.google.com/bigquery/docs/managed-disaster-recovery).

For either location type, BigQuery automatically stores copies of
your data in two different zones within a single region in the selected
location. Multi-regions are considered separate from other regions, even when
located within the same zone. For more information about data availability and
durability, see [Disaster
planning](https://docs.cloud.google.com/bigquery/docs/reliability-intro#disaster_planning).

## Supported locations

BigQuery datasets can be stored in the following regions and
multi-regions. For more information about regions and zones, see
[Geography and regions](https://docs.cloud.google.com/docs/geography-and-regions).

### Regions

The following table lists the regions in the Americas where BigQuery is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Columbus, Ohio | `us-east5` |   |
| Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Las Vegas | `us-west4` |   |
| Los Angeles | `us-west2` |   |
| Mexico | `northamerica-south1` |   |
| Montréal | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Northern Virginia | `us-east4` |   |
| Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Salt Lake City | `us-west3` |   |
| São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Santiago | `southamerica-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| South Carolina | `us-east1` |   |
| Toronto | `northamerica-northeast2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |

The following table lists the regions in Asia Pacific where BigQuery is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Bangkok | `asia-southeast3` |   |
| Delhi | `asia-south2` |   |
| Hong Kong | `asia-east2` |   |
| Jakarta | `asia-southeast2` |   |
| Melbourne | `australia-southeast2` |   |
| Mumbai | `asia-south1` |   |
| Osaka | `asia-northeast2` |   |
| Seoul | `asia-northeast3` |   |
| Singapore | `asia-southeast1` |   |
| Sydney | `australia-southeast1` |   |
| Taiwan | `asia-east1` |   |
| Tokyo | `asia-northeast1` |   |

The following table lists the regions in Europe where BigQuery is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Berlin | `europe-west10` |   |
| Finland | `europe-north1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Frankfurt | `europe-west3` |   |
| London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Milan | `europe-west8` |   |
| Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Paris | `europe-west9` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Stockholm | `europe-north2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Turin | `europe-west12` |   |
| Warsaw | `europe-central2` |   |
| Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |

The following table lists the regions in the Middle East where BigQuery is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Dammam | `me-central2` |   |
| Doha | `me-central1` |   |
| Tel Aviv | `me-west1` |   |

The following table lists the regions in Africa where BigQuery is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Johannesburg | `africa-south1` |   |

### Multi-regions

The following table lists the multi-regions where BigQuery is available. When you select a multi-region, you let BigQuery select a single region within the multi-region where your data is stored and processed.

| **Multi-region description** | **Multi-region name** |
|---|---|
| Data centers within [member states](https://europa.eu/european-union/about-eu/countries_en) of the European Union^1^ | `EU` |
| Data centers in the United States^2^ | `US` |

> [!NOTE]
> **Note:** Selecting a multi-region location does not provide cross-region replication or regional redundancy, so there is no increase in dataset availability in the event of a regional outage. Data is stored in a single region within the geographic location.

^1^ Data located in the `EU` multi-region is only
stored in one of the following locations: `europe-west1` (Belgium) or `europe-west4` (Netherlands).
The exact location in which the data is stored and processed is determined automatically by BigQuery.

^2^ Data located in the `US` multi-region is only
stored in one of the following locations: `us-central1` (Iowa),
`us-west1` (Oregon), or `us-central2` (Oklahoma). The exact
location in which the data is stored and processed is determined
automatically by BigQuery.

## BigQuery Studio locations

BigQuery Studio lets you save, share, and manage versions of code assets
such as [notebooks](https://docs.cloud.google.com/bigquery/docs/notebooks-introduction) and
[saved queries](https://docs.cloud.google.com/bigquery/docs/saved-queries-introduction).

The following table lists the regions where BigQuery Studio is available:

|   | Region description | Region name | Details |
|---|---|---|---|
| **Africa** ||||
|   | Johannesburg | `africa-south1` |   |
| **Americas** ||||
|   | Columbus | `us-east5` |   |
|   | Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Los Angeles | `us-west2` |   |
|   | Las Vegas | `us-west4` |   |
|   | Montréal | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | N. Virginia | `us-east4` |   |
|   | Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | South Carolina | `us-east1` |   |
| **Asia Pacific** ||||
|   | Hong Kong | `asia-east2` |   |
|   | Jakarta | `asia-southeast2` |   |
|   | Mumbai | `asia-south1` |   |
|   | Seoul | `asia-northeast3` |   |
|   | Singapore | `asia-southeast1` |   |
|   | Sydney | `australia-southeast1` |   |
|   | Taiwan | `asia-east1` |   |
|   | Tokyo | `asia-northeast1` |   |
| **Europe** ||||
|   | Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Frankfurt | `europe-west3` |   |
|   | London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Turin | `europe-west12` |   |
|   | Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Middle East** ||||
|   | Doha | `me-central1` |   |
|   | Dammam | `me-central2` |   |

## BigQuery Omni locations

BigQuery Omni processes queries in the same location as the dataset that contains the tables you're querying. After you create the dataset, the location cannot be changed. Your data resides within your AWS or Azure account. BigQuery Omni regions support Enterprise edition reservations and on-demand compute (analysis) pricing. For more information about editions, see [Introduction to BigQuery editions](https://docs.cloud.google.com/bigquery/docs/editions-intro).

|   | Region description | Region name | Colocated BigQuery region |
|---|---|---|---|
| **AWS** ||||
|   | AWS - US East (N. Virginia) | `aws-us-east-1` | `us-east4` |
|   | AWS - US West (Oregon) | `aws-us-west-2` | `us-west1` |
|   | AWS - Asia Pacific (Seoul) | `aws-ap-northeast-2` | `asia-northeast3` |
|   | AWS - Asia Pacific (Sydney) | `aws-ap-southeast-2` | `australia-southeast1` |
|   | AWS - Europe (Ireland) | `aws-eu-west-1` | `europe-west1` |
|   | AWS - Europe (Frankfurt) | `aws-eu-central-1` | `europe-west3` |
| **Azure** ||||
|   | Azure - East US 2 | `azure-eastus2` | `us-east4` |

## BigQuery ML locations

The following sections describe supported locations for BigQuery ML
models.

### Locations for remote models

This section contains information about supported locations for [remote models](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model), and about where remote model processing occurs.   

#### Regional locations

See the following documentation for supported locations for remote models over Google models and partner models:

- For Gemini model and embedding model supported regions, see [Google model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#google_model_endpoint_locations).
- For Claude, Llama, and Mistral AI model supported regions, see [Google Cloud partner model endpoint locations](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#genai-partner-models).

The following table shows which regions are supported for remote models over Cloud AI services and custom models deployed to Vertex AI. The column name indicates the type of remote model.

|   | Region description | Region name | Vertex AI deployed models | Cloud Natural Language API | Cloud Translation API | Cloud Vision API | Document AI API | Speech-to-Text API |
|---|---|---|---|---|---|---|---|---|
| **Americas** |||||||||
|   | Columbus, Ohio | `us-east5` |   |   |   |   |   |   |
|   | Dallas | `us-south1` | ● |   |   |   |   |   |
|   | Iowa | `us-central1` | ● |   |   |   |   | ● |
|   | Las Vegas | `us-west4` | ● |   |   |   |   |   |
|   | Los Angeles | `us-west2` | ● |   |   |   |   |   |
|   | Mexico | `northamerica-south1` |   |   |   |   |   |   |
|   | Montréal | `northamerica-northeast1` | ● |   |   |   |   |   |
|   | Northern Virginia | `us-east4` | ● |   |   |   |   |   |
|   | Oregon | `us-west1` | ● |   |   |   |   | ● |
|   | Salt Lake City | `us-west3` | ● |   |   |   |   |   |
|   | São Paulo | `southamerica-east1` | ● |   |   |   |   |   |
|   | Santiago | `southamerica-west1` |   |   |   |   |   |   |
|   | South Carolina | `us-east1` | ● |   |   |   |   | ● |
|   | Toronto | `northamerica-northeast2` | ● |   |   |   |   |   |
| **Europe** |||||||||
|   | Belgium | `europe-west1` | ● |   |   |   |   | ● |
|   | Finland | `europe-north1` |   |   |   |   |   |   |
|   | Frankfurt | `europe-west3` | ● |   |   |   |   | ● |
|   | London | `europe-west2` | ● |   |   |   |   | ● |
|   | Madrid | `europe-southwest1` |   |   |   |   |   |   |
|   | Milan | `europe-west8` | ● |   |   |   |   |   |
|   | Netherlands | `europe-west4` | ● |   |   |   |   | ● |
|   | Paris | `europe-west9` | ● |   |   |   |   |   |
|   | Stockholm | `europe-north2` |   |   |   |   |   |   |
|   | Turin | `europe-west12` |   |   |   |   |   |   |
|   | Warsaw | `europe-central2` | ● |   |   |   |   |   |
|   | Zürich | `europe-west6` | ● |   |   |   |   |   |
| **Asia Pacific** |||||||||
|   | Bangkok | `asia-southeast3` |   |   |   |   |   |   |
|   | Delhi | `asia-south2` |   |   |   |   |   |   |
|   | Hong Kong | `asia-east2` | ● |   |   |   |   |   |
|   | Jakarta | `asia-southeast2` | ● |   |   |   |   |   |
|   | Melbourne | `australia-southeast2` |   |   |   |   |   |   |
|   | Mumbai | `asia-south1` | ● |   |   |   |   | ● |
|   | Osaka | `asia-northeast2` |   |   |   |   |   |   |
|   | Seoul | `asia-northeast3` | ● |   |   |   |   |   |
|   | Singapore | `asia-southeast1` | ● |   |   |   |   | ● |
|   | Sydney | `australia-southeast1` | ● |   |   |   |   | ● |
|   | Taiwan | `asia-east1` | ● |   |   |   |   |   |
|   | Tokyo | `asia-northeast1` | ● |   |   |   |   | ● |
| **Middle East** |||||||||
|   | Dammam | `me-central2` |   |   |   |   |   |   |
|   | Doha | `me-central1` |   |   |   |   |   |   |
|   | Tel Aviv | `me-west1` | ● |   |   |   |   |   |

If the dataset in which you are creating the remote model is in a single region,
the Vertex AI model endpoint must be in the same region. If
you specify the model endpoint URL, use the endpoint in the same region
as the dataset. For example, if the dataset is in the `us-central1` region, then
specify the endpoint
`https://us-central1-aiplatform.googleapis.com/v1/projects/myproject/locations/us-central1/publishers/google/models/<target_model>`.
If you specify the model name, BigQuery ML automatically
chooses the endpoint in the correct region.

#### Multi-regional locations

Multi-regional support for remote models is as follows:

- Gemini models are supported in the `US` and `EU` multi-regions.
- Claude, Llama, and Mistral AI models in the `US` multi-region can use the Vertex AI endpoint for any single region within the `US` multi-region. Claude, Llama, and Mistral AI models in the `EU` multi-region can use the Vertex AI endpoint for any single region within the `EU` multi-region except for `eu-west2` and `eu-west6`.
- Vertex AI deployed models aren't supported in either multi-region.
- [Cloud AI services](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-remote-model-service) are supported in the `US` and `EU` multi-regions.

If the dataset in which you are creating the remote model is in a multi-region,
then the Vertex AI model endpoint must be in a region within
that multi-region. For example, if the dataset is in the `eu` multi-region,
then you could specify the URL for the `europe-west1` region endpoint,
`https://europe-west1-aiplatform.googleapis.com/v1/projects/myproject/locations/europe-west1/publishers/google/models/<target_model>`.
If you specify the model name instead of the endpoint URL,
BigQuery ML defaults to using the `europe-west4` endpoint for
datasets in the `eu` multi-region, and to using the `us-central1` endpoint for
datasets in the `us` multi-region.

#### Global endpoint

For
[supported Gemini models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#supported_models),
you can specify the
[global endpoint](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/locations#use_the_global_endpoint).

The global endpoint covers the entire world and provides
higher availability and reliability than a single region. Using
the global endpoint for your requests can improve overall
availability while reducing resource exhausted (429) errors, which occur
when you exceed your quota for a regional endpoint.
If you want to use Gemini 2.0+ in a region where it isn't
available, you can avoid migrating your data to a different region by
using the global endpoint instead. You can only use a model deployed to
the global endpoint with the `AI.GENERATE_TEXT` function.
Don't use the global endpoint if you have requirements for the data processing location, because when you use the global endpoint, you can't control or know the region where your processing requests are handled.

#### Processing locations for Google models and partner models

For information about processing locations used by Google models hosted in
Vertex AI, see
[ML processing for Google Cloud models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/data-residency#ml-processing-google-models).
This information covers models deployed to regions or multi-regions. Models that use the global
endpoint don't guarantee any particular processing location.

For information about processing locations used by partner models hosted in
Vertex AI, see
[ML processing for Google Cloud partner models](https://docs.cloud.google.com/vertex-ai/generative-ai/docs/learn/data-residency#ml-processing-partner-models).

### Locations for non-remote models

This section contains information about supported locations for [models](https://docs.cloud.google.com/bigquery/docs/bqml-introduction#supported_models) other than remote models, and about where model processing occurs.   

#### Regional locations

The following table contains information about supported locations for all model types other than remote models:

|   | Region description | Region name | Imported models | Built-in model training | DNN/Autoencoder/ Boosted Tree/ Wide-and-Deep models training | AutoML model training | Hyperparameter tuning | Vertex AI Model Registry integration |
|---|---|---|---|---|---|---|---|---|---|---|
| **Americas** |||||||||
|   | Columbus, Ohio | `us-east5` | ● | ● |   |   |   |   |
|   | Dallas | `us-south1` | ● | ● |   |   |   |   |   |
|   | Iowa | `us-central1` | ● | ● | ● | ● | ● | ● |
|   | Las Vegas | `us-west4` | ● | ● |   | ● |   | ● |
|   | Los Angeles | `us-west2` | ● | ● | ● |   |   | ● |
|   | Mexico | `northamerica-south1` | ● | ● |   |   |   |   |   |   |
|   | Montréal | `northamerica-northeast1` | ● | ● | ● | ● | ● | ● |
|   | Northern Virginia | `us-east4` | ● | ● | ● | ● | ● | ● |
|   | Oregon | `us-west1` | ● | ● | ● |   | ● | ● |
|   | Salt Lake City | `us-west3` | ● | ● | ● |   |   |   |
|   | São Paulo | `southamerica-east1` | ● | ● | ● | ● |   |   |
|   | Santiago | `southamerica-west1` | ● | ● |   |   |   |   |
|   | South Carolina | `us-east1` | ● | ● | ● |   | ● | ● |
|   | Toronto | `northamerica-northeast2` | ● | ● |   | ● |   |   |
| **Europe** |||||||||
|   | Belgium | `europe-west1` | ● | ● | ● | ● | ● | ● |
|   | Berlin | `europe-west10` | ● | ● |   |   |   |   |
|   | Finland | `europe-north1` | ● | ● | ● |   |   |   |
|   | Frankfurt | `europe-west3` | ● | ● | ● | ● | ● | ● |
|   | London | `europe-west2` | ● | ● | ● | ● | ● | ● |
|   | Madrid | `europe-southwest1` | ● | ● |   |   |   |   |
|   | Milan | `europe-west8` | ● | ● |   |   |   |   |
|   | Netherlands | `europe-west4` | ● | ● | ● | ● | ● | ● |
|   | Paris | `europe-west9` | ● | ● |   |   |   |   |
|   | Stockholm | `europe-north2` | ● | ● |   |   |   |   |
|   | Turin | `europe-west12` |   | ● |   |   |   |   |
|   | Warsaw | `europe-central2` | ● | ● |   |   |   |   |
|   | Zürich | `europe-west6` | ● | ● | ● | ● | ● | ● |
| **Asia Pacific** |||||||||
|   | Bangkok | `asia-southeast3` | ● | ● |   |   |   |   |
|   | Delhi | `asia-south2` | ● | ● |   |   |   |   |
|   | Hong Kong | `asia-east2` | ● | ● | ● | ● | ● | ● |
|   | Jakarta | `asia-southeast2` | ● | ● |   |   |   | ● |
|   | Melbourne | `australia-southeast2` | ● | ● |   |   |   |   |
|   | Mumbai | `asia-south1` | ● | ● | ● | ● |   | ● |
|   | Osaka | `asia-northeast2` | ● | ● | ● |   |   |   |
|   | Seoul | `asia-northeast3` | ● | ● | ● | ● | ● | ● |
|   | Singapore | `asia-southeast1` | ● | ● | ● | ● | ● | ● |
|   | Sydney | `australia-southeast1` | ● | ● | ● | ● | ● | ● |
|   | Taiwan | `asia-east1` | ● | ● | ● | ● | ● | ● |
|   | Tokyo | `asia-northeast1` | ● | ● | ● | ● | ● | ● |
| **Middle East** |||||||||
|   | Dammam | `me-central2` |   | ● |   |   |   |   |
|   | Doha | `me-central1` |   | ● |   |   |   |   |
|   | Tel Aviv | `me-west1` | ● | ● |   |   |   |   |
| **Africa** |||||||||
|   | Johannesburg | `africa-south1` | ● | ● |   |   |   |   |

#### Multi-regional locations

All supported models other than remote models are supported in the `US` and
`EU` multi-regions.

Data located in the `EU` multi-region is not stored in the `europe-west2`
(London) or `europe-west6` (Zürich) data centers.

Vertex AI Model Registry integration is supported only for single region integrations. If
you send a multi-region BigQuery ML model to the Model Registry,
then it is converted to a regional model in Vertex AI.
A BigQuery ML multi-region US model is synced to Vertex AI
`us-central1` and a BigQuery ML multi-region EU model is synced to
Vertex AI `europe-west4`. For single region models, there are
no changes.

#### Processing locations

For models other than remote models, BigQuery ML processes and stages data in the
same location as the dataset that contains the data.

BigQuery ML stores your data in the selected location in
accordance with the [Service Specific Terms](https://cloud.google.com/terms/service-terms#13-google-bigquery-service).

## BigQuery SQL translator locations

When migrating data from your legacy data warehouse into BigQuery,
you can use several SQL translators to translate your SQL queries into GoogleSQL
or other supported SQL dialects. These include the [interactive SQL translator](https://docs.cloud.google.com/bigquery/docs/interactive-sql-translator),
the [SQL translation API](https://docs.cloud.google.com/bigquery/docs/api-sql-translator), and the [batch SQL translator](https://docs.cloud.google.com/bigquery/docs/batch-sql-translator).

The BigQuery SQL translators are available in the following
processing locations:

|   | **Region description** | **Region name** | **Details** |
|---|---|---|---|
| **Asia Pacific** ||||
|   | Bangkok | `asia-southeast3` |   |
|   | Delhi | `asia-south2` |   |
|   | Hong Kong | `asia-east2` |   |
|   | Jakarta | `asia-southeast2` |   |
|   | Melbourne | `australia-southeast2` |   |
|   | Mumbai | `asia-south1` |   |
|   | Osaka | `asia-northeast2` |   |
|   | Seoul | `asia-northeast3` |   |
|   | Singapore | `asia-southeast1` |   |
|   | Sydney | `australia-southeast1` |   |
|   | Taiwan | `asia-east1` |   |
|   | Tokyo | `asia-northeast1` |   |
| **Europe** ||||
|   | Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Berlin | `europe-west10` |   |
|   | EU multi-region | `eu` |
|   | Finland | `europe-north1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Frankfurt | `europe-west3` |   |
|   | London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Milan | `europe-west8` |   |
|   | Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Paris | `europe-west9` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Stockholm | `europe-north2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Turin | `europe-west12` |   |
|   | Warsaw | `europe-central2` |   |
|   | Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Americas** ||||
|   | Columbus, Ohio | `us-east5` |   |
|   | Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Las Vegas | `us-west4` |   |
|   | Los Angeles | `us-west2` |   |
|   | Mexico | `northamerica-south1` |   |
|   | Northern Virginia | `us-east4` |   |
|   | Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Québec | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Salt Lake City | `us-west3` |   |
|   | Santiago | `southamerica-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | South Carolina | `us-east1` |   |
|   | Toronto | `northamerica-northeast2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | US multi-region | `us` |
| **Africa** ||||
|   | Johannesburg | `africa-south1` |   |
| **MiddleEast** ||||
|   | Dammam | `me-central2` |   |
|   | Doha | `me-central1` |   |
|   | Israel | `me-west1` |   |

## BigQuery continuous query locations

The following table lists the regions where continuous queries are supported:

|   | Region description | Region name | Details |
|---|---|---|---|
| **Americas** ||||
|   | US multi-region | `us` |
|   | Columbus | `us-east5` |   |
|   | Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Las Vegas | `us-west4` |   |
|   | Los Angeles | `us-west2` |   |
|   | Mexico | `northamerica-south1` |   |
|   | Montréal | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Northern Virginia | `us-east4` |   |
|   | Oklahoma | `us-central2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Salt Lake City | `us-west3` |   |
|   | Santiago | `southamerica-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | South Carolina | `us-east1` |   |
|   | Toronto | `northamerica-northeast2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Asia Pacific** ||||
|   | Delhi | `asia-south2` |   |
|   | Hong Kong | `asia-east2` |   |
|   | Jakarta | `asia-southeast2` |   |
|   | Melbourne | `australia-southeast2` |   |
|   | Mumbai | `asia-south1` |   |
|   | Osaka | `asia-northeast2` |   |
|   | Seoul | `asia-northeast3` |   |
|   | Singapore | `asia-southeast1` |   |
|   | Sydney | `australia-southeast1` |   |
|   | Taiwan | `asia-east1` |   |
|   | Tokyo | `asia-northeast1` |   |
| **Europe** ||||
|   | EU multi-region | `eu` |
|   | Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Berlin | `europe-west10` |   |
|   | Finland | `europe-north1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Frankfurt | `europe-west3` |   |
|   | London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Milan | `europe-west8` |   |
|   | Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Paris | `europe-west9` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Stockholm | `europe-north2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Turin | `europe-west12` |   |
|   | Warsaw | `europe-central2` |   |
|   | Zurich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Middle East** ||||
|   | Doha | `me-central1` |   |
|   | Dammam | `me-central2` |   |
|   | Tel Aviv | `me-west1` |   |
| **Africa** ||||
|   | Johannesburg | `africa-south1` |   |

## BigQuery partition and cluster recommender locations

The [BigQuery partitioning and clustering recommender](https://docs.cloud.google.com/bigquery/docs/manage-partition-cluster-recommendations)
generates partition or cluster
recommendations to optimize your BigQuery tables.

The partitioning and clustering recommender is available in the following
processing locations:

|   | **Region description** | **Region name** | **Details** |
|---|---|---|---|
| **Asia Pacific** ||||
|   | Delhi | `asia-south2` |   |
|   | Hong Kong | `asia-east2` |   |
|   | Jakarta | `asia-southeast2` |   |
|   | Mumbai | `asia-south1` |   |
|   | Osaka | `asia-northeast2` |   |
|   | Seoul | `asia-northeast3` |   |
|   | Singapore | `asia-southeast1` |   |
|   | Sydney | `australia-southeast1` |   |
|   | Taiwan | `asia-east1` |   |
|   | Tokyo | `asia-northeast1` |   |
| **Europe** ||||
|   | Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Berlin | `europe-west10` |   |
|   | EU multi-region | `eu` |
|   | Frankfurt | `europe-west3` |   |
|   | London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| **Americas** ||||
|   | Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Las Vegas | `us-west4` |   |
|   | Los Angeles | `us-west2` |   |
|   | Montréal | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Northern Virginia | `us-east4` |   |
|   | Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Salt Lake City | `us-west3` |   |
|   | São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | Toronto | `northamerica-northeast2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
|   | US multi-region | `us` |

## BigQuery sharing locations

BigQuery sharing (formerly Analytics Hub) is available in the following regions and
multi-regions.

#### Regions

The following table lists the regions in the Americas where sharing is available.

| Region description | Region name | Details |
|---|---|---|
| Columbus, Ohio | `us-east5` |   |
| Dallas | `us-south1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Iowa | `us-central1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Las Vegas | `us-west4` |   |
| Los Angeles | `us-west2` |   |
| Mexico | `northamerica-south1` |   |
| Montréal | `northamerica-northeast1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Northern Virginia | `us-east4` |   |
| Oklahoma | `us-central2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Oregon | `us-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Salt Lake City | `us-west3` |   |
| São Paulo | `southamerica-east1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Santiago | `southamerica-west1` |   |
| South Carolina | `us-east1` |   |
| Toronto | `northamerica-northeast2` |   |

The following table lists the regions in Asia Pacific where sharing is available.

| Region description | Region name | Details |
|---|---|---|
| Delhi | `asia-south2` |   |
| Hong Kong | `asia-east2` |   |
| Jakarta | `asia-southeast2` |   |
| Melbourne | `australia-southeast2` |   |
| Mumbai | `asia-south1` |   |
| Osaka | `asia-northeast2` |   |
| Seoul | `asia-northeast3` |   |
| Singapore | `asia-southeast1` |   |
| Sydney | `australia-southeast1` |   |
| Taiwan | `asia-east1` |   |
| Tokyo | `asia-northeast1` |   |

The following table lists the regions in Europe where sharing is available.

| Region description | Region name | Details |
|---|---|---|
| Belgium | `europe-west1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Berlin | `europe-west10` |   |
| Finland | `europe-north1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Frankfurt | `europe-west3` |   |
| London | `europe-west2` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Madrid | `europe-southwest1` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Milan | `europe-west8` |   |
| Netherlands | `europe-west4` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Paris | `europe-west9` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |
| Turin | `europe-west12` |   |
| Warsaw | `europe-central2` |   |
| Zürich | `europe-west6` | ![leaf icon](https://cloud.google.com/sustainability/region-carbon/gleaf.svg) [Low CO~2~](https://cloud.google.com/sustainability/region-carbon#region-picker) |

The following table lists the regions in the Middle East where sharing is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Dammam | `me-central2` |   |
| Doha | `me-central1` |   |
| Tel Aviv | `me-west1` |   |

The following table lists the regions in Africa where sharing is available.

| **Region description** | **Region name** | **Details** |
|---|---|---|
| Johannesburg | `africa-south1` |   |

#### Multi-regions

The following table lists the multi-regions where sharing is available.

| Multi-region description | Multi-region name |
|---|---|
| Data centers within [member states](https://europa.eu/european-union/about-eu/countries_en) of the European Union^1^ | `EU` |
| Data centers in the United States | `US` |

^1^ Data located in the `EU` multi-region is not
stored in the `europe-west2` (London) or `europe-west6` (Zürich) data
centers.

#### Omni regions

The following table lists the Omni where sharing is available.

|   | Omni region description | Omni region name |
|---|---|---|
| **AWS** |||
|   | AWS - US East (N. Virginia) | `aws-us-east-1` |
|   | AWS - US West (Oregon) | `aws-us-west-2` |
|   | AWS - Asia Pacific (Seoul) | `aws-ap-northeast-2` |
|   | AWS - Asia Pacific (Sydney) | `aws-ap-southeast-2` |
|   | AWS - Europe (Ireland) | `aws-eu-west-1` |
|   | AWS - Europe (Frankfurt) | `aws-eu-central-1` |
| **Azure** |||
|   | Azure - East US 2 | `azure-eastus2` |

## Specify locations

To ensure that BigQuery queries are stored in a specific region
or multi-region, specify the location with the job request. Specifying the
location ensures that the query is run in the correct location when you use the
global BigQuery endpoint.

If you don't specify the location, queries might be temporarily stored in
BigQuery router logs when the query is used for determining the
processing location in BigQuery.

If the [project](https://docs.cloud.google.com/bigquery/docs/resource-hierarchy#projects) has a
capacity-based reservation in a region other than the `US` and the query does
not reference any tables or other resources contained within datasets, then you
must explicitly specify the location of the capacity-based reservation when
submitting the job. Capacity-based commitments are tied to a location, such as
`US` or `EU`. If you run a job outside the location of your capacity, pricing
for that job automatically shifts to on-demand pricing.

You can specify the location to run a job explicitly in the following ways:

- When you query data using the Google Cloud console in the query editor, click **More \> Query
  settings** , expand **Advanced options** , and then select your **Data
  location**.
- When you write a SQL query, set the [`@@location` system variable](https://docs.cloud.google.com/bigquery/docs/reference/system-variables) in the first statement of your query.
- When you use the bq command-line tool, supply the `--location` [global flag](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#global_flags) and set the value to your location.
- When you use the API, specify your region in the `location` property in the `jobReference` section of the [job resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs).

BigQuery returns an error if the specified location does not match
the location of the datasets in the request. The location of every dataset
involved in the request, including those read from and those written to, must
match the location of the job as inferred or specified.

Single-region locations don't match multi-region locations, even where the
single-region location is contained within the multi-region location. Therefore,
a query will be run as a [global query](https://docs.cloud.google.com/bigquery/docs/global-queries) if the location includes both a single-region location
and a multi-region location. For example, if a job's location is set to `US`,
the job will be a global query if it references a dataset in `us-central1`. Likewise, a job
that references one dataset in `US` and another dataset in `us-central1` will be a global query. This is also true for `JOIN` statements with tables in both a region and a multi-region.

[Dynamic queries](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/procedural-language#execute_immediate)
aren't parsed until they execute, so they can't be used to automatically
determine the region of a query.

## Default location

If you don't [explicitly specify a location](https://docs.cloud.google.com/bigquery/docs/locations#specify_locations),
the location is determined in one of the following ways:

- The location of the datasets referenced in the request. For example, if a query references a table or view in a dataset stored in the `asia-northeast1` region, the query job runs in `asia-northeast1`.
- The region specified for a connection referenced in a request.
- The location of a destination table.

If the location isn't explicitly specified, and it can't be determined from the
resources in the request, the default location is used. If default location
isn't set, the job runs in the `US` multi-region.

For more information about configuring the default location, see [Specify global
settings](https://docs.cloud.google.com/bigquery/docs/default-configuration#global-settings).

## Locations, reservations, and jobs

Capacity commitments are a regional resource. When you buy slots, those slots
are limited to a specific region or multi-region. If your only capacity
commitment is in the `EU` then you can't create a reservation in the `US`. When
you create a reservation, you specify a location (region) and a number of slots.
Those slots are pulled from your capacity commitment in that region.

Likewise, when you run a job in a region, it only uses a reservation if the
location of the job matches the location of a reservation, unless the job is a [global query](https://docs.cloud.google.com/bigquery/docs/global-queries).
For example, if you assign a reservation to a project in the `EU` and run a query in that project
on a dataset located in the `US`, then that query is not run on your `EU`
reservation. In the absence of any `US` reservation, the job is run as
on-demand.

## Location considerations

When you choose a location for your data, consider the following:

### Cloud Storage

You can interact with Cloud Storage data using BigQuery in the
following ways:

- [Query Cloud Storage data](https://docs.cloud.google.com/bigquery/docs/locations#query-storage-data-location) using BigLake or non-BigLake external tables
- [Load Cloud Storage data into BigQuery](https://docs.cloud.google.com/bigquery/docs/locations#load-data-location-considerations)

#### Query Cloud Storage data

When you query data in Cloud Storage by using a
[BigLake](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-using-biglake) or a
[non-BigLake external table](https://docs.cloud.google.com/bigquery/docs/query-cloud-storage-data),
the data you query must be colocated with your BigQuery dataset,
otherwise the query incurs [data transfer charges](https://cloud.google.com/storage/pricing#network-buckets).
For example:

- [Single region bucket](https://docs.cloud.google.com/storage/docs/locations#location-r): If your BigQuery dataset is in the Warsaw (`europe-central2`) region, the corresponding Cloud Storage bucket must also be in the Warsaw region, or any Cloud Storage dual-region that includes Warsaw.
  If your BigQuery dataset is in the `US` multi-region,
  then the Cloud Storage bucket can be in the Iowa (`us-central1`) single region, or any dual-region that includes Iowa.
  Queries from any other single region incur data transfer charges, even if
  the bucket is in a location that is contained within the multi-region of the dataset.
  For example, if the external tables are in the `US` multi-region and the
  Cloud Storage bucket is in Oregon (`us-west1`), the job incurs
  data transfer charges.

  If your BigQuery dataset is in the `EU` multi-region,
  then the Cloud Storage bucket can be in the Netherlands (`europe-west4`)
  single region or any dual-region that includes Netherlands (`europe-west4`). Queries from any other single region incur data transfer fees, even if the bucket
  is in a location that is contained within the multi-region of the dataset.
  For example, if the external tables are in the `EU` multi-region and the
  Cloud Storage bucket is in Warsaw (`europe-central2`), the job incurs data transfer charges.
- [Dual-region bucket](https://docs.cloud.google.com/storage/docs/locations#location-dr): If your
  BigQuery dataset is in the Tokyo (`asia-northeast1`) region,
  the corresponding Cloud Storage bucket must be in the Tokyo region, or
  in a dual-region that includes Tokyo, like the `ASIA1` dual-region.

  If the Cloud Storage bucket is in the `NAM4` dual-region or any dual-region that
  includes the Iowa(`us-central1`) region, the corresponding BigQuery
  dataset can be in the `US` multi-region or in the Iowa(`us-central1`).

  If Cloud Storage bucket is in the `EUR4` dual-region or any dual-region that
  includes the Netherlands (`europe-west4`) region, the corresponding BigQuery
  dataset can be in the `EU` multi-region or in the Netherlands (`europe-west4`).
- [Multi-region bucket](https://docs.cloud.google.com/storage/docs/locations#location-mr): Using multi-region
  dataset locations with multi-region Cloud Storage buckets is
  **not** recommended for external tables, because external query performance
  depends on minimal latency and optimal network bandwidth.

  If your BigQuery dataset is in the `US` multi-region, the
  corresponding Cloud Storage bucket must be in a dual-region that includes Iowa (`us-central1`), like the `NAM4`
  dual-region, or in a custom dual-region that includes Iowa (`us-central1`).

  If your BigQuery dataset is in the `EU` multi-region, the
  corresponding Cloud Storage bucket must be in a dual-region that includes Netherlands (`europe-west4`), like the `EUR4`
  dual-region, or in a custom dual-region that includes Netherlands (`europe-west4`) .

For more information about supported Cloud Storage locations, see
[Bucket locations](https://docs.cloud.google.com/storage/docs/bucket-locations) in the
Cloud Storage documentation.

#### Load Cloud Storage data into BigQuery

When you load data from Cloud Storage, the data that you load must be colocated
with your BigQuery dataset, otherwise the load job incurs data transfer charges.

For more information about load data transfer charges, see the [Query Cloud Storage data](https://docs.cloud.google.com/bigquery/docs/locations#query-storage-data-location)
section, as the same guidance applies to both batch loads and queries.

For more information, see [Batch loading data](https://docs.cloud.google.com/bigquery/docs/batch-loading-data).

### Bigtable

You must consider location when querying data from
Bigtable or exporting data to Bigtable.

#### Query Bigtable data

When you [query data in Bigtable](https://docs.cloud.google.com/bigquery/docs/external-data-bigtable)
through a BigQuery [external table](https://docs.cloud.google.com/bigquery/docs/external-tables),
your Bigtable instance must be in the same location as your
BigQuery dataset:

- Single region: If your BigQuery dataset is in the Belgium (`europe-west1`) regional location, the corresponding Bigtable instance must be in the Belgium region.
- Multi-region: Because external query performance depends on minimal latency and optimal network bandwidth, using multi-region dataset locations is **not** recommended for external tables on Bigtable.

For more information about supported Bigtable locations, see
[Bigtable locations](https://docs.cloud.google.com/bigtable/docs/locations).

#### Export data to Bigtable

- If your BigQuery dataset is in a multi-region, your [Bigtable app profile](https://docs.cloud.google.com/bigtable/docs/app-profiles) must be configured to route data to a Bigtable cluster within that multi-region. For example, if your BigQuery dataset is in the `US` multi-region, the Bigtable cluster can be located in the `us-west1` (Oregon) region, which is within the United States.
- If your BigQuery dataset is in a single region, your [Bigtable app profile](https://docs.cloud.google.com/bigtable/docs/app-profiles) must be configured to route data to a Bigtable cluster in the same region. For example, if your BigQuery dataset is in the `asia-northeast1` (Tokyo) region, your Bigtable cluster must also be in the `asia-northeast1` (Tokyo) region.

### Google Drive

Location considerations do not apply to [Google Drive](https://docs.cloud.google.com/bigquery/external-data-drive)
external data sources.

### Cloud SQL

When you [query data in Cloud SQL](https://docs.cloud.google.com/bigquery/docs/cloud-sql-federated-queries)
through a BigQuery [federated query](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro),
your Cloud SQL instance must be in the same location as your
BigQuery dataset.

- Single region: If your BigQuery dataset is in the Belgium (`europe-west1`) regional location, the corresponding Cloud SQL instance must be in the Belgium region.
- Multi-region: If your BigQuery dataset is in the `US` multi-region, the corresponding Cloud SQL instance must be in a single region in the US geographic area.

For more information about supported Cloud SQL locations, see
[Cloud SQL locations](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).

### Spanner

When you [query data in Spanner](https://docs.cloud.google.com/bigquery/docs/spanner-federated-queries)
through a BigQuery [federated query](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro),
your Spanner instance must be in the same location as your
BigQuery dataset.

- Single region: If your BigQuery dataset is in the Belgium (`europe-west1`) regional location, the corresponding Spanner instance must be in the Belgium region.
- Multi-region: If your BigQuery dataset is in the `US` multi-region, the corresponding Spanner instance must be in a single region in the US geographic area.

For more information about supported Spanner locations, see
[Spanner locations](https://docs.cloud.google.com/bigquery/docs/federated-queries-intro#supported_regions).

### Analysis tools

Colocate your BigQuery dataset with your [analysis tools](https://docs.cloud.google.com/bigquery/docs/query-overview):

- [Managed Service for Apache Spark](https://docs.cloud.google.com/dataproc/docs/concepts/overview): When you query BigQuery datasets using a [BigQuery connector](https://docs.cloud.google.com/dataproc/docs/concepts/connectors/bigquery), your BigQuery dataset should be colocated with your Managed Service for Apache Spark cluster.
Managed Service for Apache Spark is supported in all [Compute Engine locations](https://docs.cloud.google.com/compute/docs/regions-zones#available).
- [Vertex AI Workbench](https://docs.cloud.google.com/vertex-ai/docs/workbench/introduction): When you query BigQuery datasets using [Jupyter notebooks](https://docs.cloud.google.com/bigquery/docs/programmatic-analysis#jupyter_notebooks) in Vertex AI Workbench, your BigQuery dataset should be colocated with your Vertex AI Workbench instance.
View the [supported
Vertex AI Workbench locations](https://docs.cloud.google.com/vertex-ai/docs/general/locations#vertex-ai-workbench-locations).

### Data management plans

Develop a data management plan:

- If you choose a regional storage resource such as a BigQuery dataset or a Cloud Storage bucket, develop a plan for [geographically managing your data](https://docs.cloud.google.com/docs/geography-and-regions#geographic_management_of_data).

## Restrict locations

You can restrict the locations in which your datasets can be created by using
the [Organization Policy Service](https://docs.cloud.google.com/resource-manager/docs/organization-policy/overview).
For more information, see [Restricting resource
locations](https://docs.cloud.google.com/resource-manager/docs/organization-policy/defining-locations) and
[Resource locations supported
services](https://docs.cloud.google.com/resource-manager/docs/organization-policy/defining-locations-supported-services#bigquery).

## Dataset security

To control access to datasets in BigQuery, see
[Controlling access to datasets](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam).
For information about data encryption, see [Encryption at rest](https://docs.cloud.google.com/bigquery/docs/encryption-at-rest).

## What's next

- Learn how to [create datasets](https://docs.cloud.google.com/bigquery/docs/datasets).
- Learn about [loading data into BigQuery](https://docs.cloud.google.com/bigquery/docs/loading-data).
- Learn about BigQuery [pricing](https://cloud.google.com/bigquery/pricing).
- Learn about [global queries](https://docs.cloud.google.com/bigquery/docs/global-queries).
- [View all the Google Cloud services available in locations worldwide](https://docs.cloud.google.com/about/locations#region).
- [Explore additional location-based concepts](https://docs.cloud.google.com/docs/geography-and-regions), such as zones, that apply to other Google Cloud services.