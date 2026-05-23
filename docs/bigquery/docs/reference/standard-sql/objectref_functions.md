GoogleSQL for BigQuery supports the following ObjectRef functions.

This topic includes functions that let you create and interact with
[`ObjectRef`](https://docs.cloud.google.com/bigquery/docs/work-with-objectref) and
[`ObjectRefRuntime`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objectrefruntime)
values.

An `ObjectRef` value represents a Cloud Storage object, including the object
URI, size, type, and similar metadata. It also contains an authorizer, which
identifies the
[Cloud resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection)
to use to access the Cloud Storage object from BigQuery. An `ObjectRef`
value is a `STRUCT` that has the following format:

    STRUCT {
      uri string,  // Cloud Storage object URI
      version string,  // Cloud Storage object version
      authorizer string,  // Cloud resource connection to use for object access
      details json {  // Cloud Storage managed object metadata
        gcs_metadata json {
          "content_type": string,  // for example, "image/png"
          "md5_hash": string,  // for example, "d9c38814e44028bf7a012131941d5631"
          "size": number,  // for example, 23000
          "updated": number  // for example, 1741374857000000
        }
      }
    }

The fields in the
`gcs_metadata` JSON refer to the [object metadata](https://docs.cloud.google.com/storage/docs/metadata)
for a Cloud Storage object.

## Function list

| Name | Summary |
|---|---|
| [`OBJ.FETCH_METADATA`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objfetch_metadata) | Fetches Cloud Storage metadata for a partially populated `ObjectRef` value. |
| [`OBJ.GET_ACCESS_URL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_access_url) | Returns access URLs for a Cloud Storage object. |
| [`OBJ.GET_READ_URL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_read_url) | Returns a read URL and status for a Cloud Storage object. |
| [`OBJ.MAKE_REF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objmake_ref) | Creates an `ObjectRef` value that contains reference information for a Cloud Storage object. |

## `OBJ.FETCH_METADATA`

    OBJ.FETCH_METADATA(
      objectref
    )

    OBJ.FETCH_METADATA(
      ARRAY<objectref>
    )

**Description**

The `OBJ.FETCH_METADATA` function returns Cloud Storage metadata for a partially
populated
[`ObjectRef` value](https://docs.cloud.google.com/bigquery/docs/work-with-objectref).

This function lets the `ObjectRef` value use either
[direct access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#direct-access) or
[delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access) to the
object.

This function still succeeds if there is a problem fetching metadata. In this
case, the `details` field contains an `error` field with the
error message, as shown in the following example:

    {
      "details": {
        "errors": [{
          "code":400,
          "message":"Connection credential for projects/myproject/locations/us/connections/connection1 cannot be used. Either the connection does not exist, or the user does not have sufficient permissions to use it.",
          "source":"OBJ.FETCH_METADATA",
        }]
      }
    }

**Definitions**

- `objectref`: A partially populated `ObjectRef` value, in which the `uri` field is populated, the `authorizer` field is optional, and the `details` field is not populated.

**Output**

If your input is a single `ObjectRef` value, then the function returns
a fully populated `ObjectRef` value. The metadata is provided in the `details`
field of the returned `ObjectRef` value.

If your input is an array of `ObjectRef` values, then the function returns
an array of fully populated `ObjectRef` values. The metadata
is provided in the `details` field of each returned `ObjectRef` value.

**Examples**

The following query populates the metadata fields for an `ObjectRef` value
based on a PNG object in a publicly available Cloud Storage bucket:

    SELECT
      OBJ.FETCH_METADATA(
        OBJ.MAKE_REF("gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/images/aquaclear-aquarium-background-poster.png", "us.connection1")
      ) AS obj;

    /*---+---+---+---+
     | obj.uri                     | obj.version      | obj.authorizer           | obj.details                                      |
     +---+---+---+---+
     | gs://cloud-samples-data/... | 1742492679764550 | myproject.us.connection1 | {"gcs_metadata":                                 |
     |                             |                  |                          |   {"content_type":"image/png",                   |
     |                             |                  |                          |   "md5_hash":"e83227b9915e26bf7a42a38f7ce8d415", |
     |                             |                  |                          |   "size":1629498,                                |
     |                             |                  |                          |   "updated":1742492679000000                     |
     |                             |                  |                          |   }                                              |
     |                             |                  |                          | }                                                |
     +---+---+---+---*/

The following query populates the metadata fields for each `ObjectRef` value in
the input array. The result is a single row that contains an array of `ObjectRef`
values.

    SELECT
      OBJ.FETCH_METADATA(
        [
          OBJ.MAKE_REF("gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/images/aquaclear-aquarium-background-poster.png", "us.connection1"),
          OBJ.MAKE_REF("gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/images/aquaclear-aquarium-fish-net.png", "us.connection1")
        ]
      ) AS obj;

    /*---+---+---+---+
     | obj.uri                     | obj.version      | obj.authorizer           | obj.details                                      |
     +---+---+---+---+
     | gs://cloud-samples-data/... | 1742492679764550 | myproject.us.connection1 | {"gcs_metadata":                                 |
     |                             |                  |                          |   {"content_type":"image/png",                   |
     |                             |                  |                          |   "md5_hash":"e83227b9915e26bf7a42a38f7ce8d415", |
     |                             |                  |                          |   "size":1629498,                                |
     |                             |                  |                          |   "updated":1742492679000000                     |
     |                             |                  |                          |   }                                              |
     |                             |                  |                          | }                                                |
     | gs://cloud-samples-data/... | 1742492681709630 | myproject.us.connection1 | {"gcs_metadata":                                 |
     |                             |                  |                          |   {"content_type":"image/png",                   |
     |                             |                  |                          |   "md5_hash":"07715c290072a357a11fb89da940b3cf", |
     |                             |                  |                          |   "size":1163692,                                |
     |                             |                  |                          |   "updated":1742492681000000                     |
     |                             |                  |                          |   }                                              |
     |                             |                  |                          | }                                                |
     +---+---+---+---*/

**Limitations**

You can't have more than 20 Cloud resource connections in the project and
region where your query accesses object data as `ObjectRef` values.

## `OBJ.GET_ACCESS_URL`

    OBJ.GET_ACCESS_URL(
      objectref,
      mode
      [, duration]
    )

    OBJ.GET_ACCESS_URL(
      ARRAY<objectref>,
      mode
      [, duration]
    )

**Description**

The `OBJ.GET_ACCESS_URL` function returns a JSON value that contains reference
information for the input
[`ObjectRef`](https://docs.cloud.google.com/bigquery/docs/work-with-objectref)
value, and also
access URLs that you can use to read or modify the Cloud Storage object.

This function requires you to use
[delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access)
to read the object.

If the function encounters an error, the returned JSON contains a
`errors` field with the error message instead of the
`access_urls` field with the access URLs. The following example shows an
error message:

    {
      "objectref": {
        "authorizer": "myproject.us.connection1",
        "uri": "gs://mybucket/path/to/file.jpg"
      },
      "errors": [{
        "code":400,
        "message":"Connection credential for projects/myproject/locations/us/connections/connection1 cannot be used. Either the connection does not exist, or the user does not have sufficient permissions to use it.",
        "source":"OBJ.GET_ACCESS_URL",
      }]
    }

**Definitions**

- `objectref`: An `ObjectRef` value that represents a Cloud Storage object.
- `mode`: A `STRING` value that identifies the type of URL that you want to
  be returned. The following values are supported:

  - `r`: Returns a URL that lets you read the object.
  - `rw`: Returns two URLs, one that lets you read the object, and one that lets you modify the object.
- `duration`: An optional [`INTERVAL`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types#interval_type) value that specifies how long
  the generated access URLs remain valid. You can specify a value between
  30 minutes and 6 hours. For example, you could specify
  `INTERVAL 2 HOUR` to generate URLs that expire after 2 hours. The default
  value is 6 hours.

**Output**

A JSON value or array of JSON values that contains the Cloud Storage object
reference
information from the input `ObjectRef` value, and also one or more URLs that
you can use to access the Cloud Storage object.

The JSON output is returned in the `ObjectRefRuntime`
schema:

    obj_ref_runtime json {
      obj_ref json {
        uri string, // Cloud Storage object URI
        version string, // Cloud Storage object version
        authorizer string, // Cloud resource connection to use for object access
        details json { // Cloud Storage managed object metadata
          gcs_metadata json {
          }
        }
      }
      access_urls json {
        read_url string, // read-only signed url
        write_url string, // writeable signed url
        expiry_time string // the URL expiration time in YYYY-MM-DD'T'HH:MM:SS'Z' format
      }
    }

**Example**

This example returns read URLs for all of the image objects associated with
the films in the `mydataset.films` table, where the `poster` column is a
struct in the `ObjectRef` schema. The URLs expire in 45 minutes.

    SELECT
      OBJ.GET_ACCESS_URL(poster, 'r', INTERVAL 45 MINUTE) AS read_url
    FROM mydataset.films;

**Limitations**

You can't have more than 20 Cloud resource connections in the project and
region where your query accesses object data as `ObjectRef` values.

## `OBJ.GET_READ_URL`

    OBJ.GET_READ_URL(objectref)

**Description**

The `OBJ.GET_READ_URL` function returns a `STRUCT` value that contains a read
URL that you can use to read the Cloud Storage object. The URL expires after
45 minutes.

This function requires you to use
[delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access) to read
for the input `ObjectRef` value.

**Definitions**

- `objectref`: an `ObjectRef` value that represents a Cloud Storage object

**Output**

A `STRUCT` value that contains the following fields:

- `url`: a read URL that you can use to read the Cloud Storage object. If the function can't create the read URL, then this value is `NULL`.
- `status`: an error message. If the function successfully creates the read URL, then this value is `NULL`.

**Examples**

In the following example, the `mydataset.films` table has a
`STRUCT` column `poster` that contains values with the `ObjectRef` schema.
The following query returns a read URL for each of the image objects associated
with the films:

    SELECT
      OBJ.GET_READ_URL(poster) AS read_url
    FROM mydataset.films;

    /*---+---+
     | read_url.url                                                   | read_url.status |
     +---+---+
     | https://storage.googleapis.com/posters/poster-1.jpg?X-Goog-... | NULL            |
     +---+---*/

When you run this query in Studio, the `read_url.url` column
displays the images corresponding to the read URLs. To view the text of the
URLs, select the **JSON** tab in the **Query results** pane.

**Limitations**

You can't have more than 20 connections in the project and region in which
your query accesses object data as `ObjectRef` values.

## `OBJ.MAKE_REF`

This function supports the following syntaxes:

    OBJ.MAKE_REF(
      uri
      [, authorizer ]
      [, version => version_value ]
      [, details => gcs_metadata_json ]
    )

    OBJ.MAKE_REF(
      objectref_json
    )

When you use this syntax, the top-level `authorizer` argument overwrites
any `authorizer` that you specify in the `objectref` argument.

    OBJ.MAKE_REF(
      objectref,
      authorizer
    )

**Description**

Use the `OBJ.MAKE_REF` function to create an
[`ObjectRef` value](https://docs.cloud.google.com/bigquery/docs/work-with-objectref)
that contains reference information for a Cloud Storage object.
You can use this function in workflows similar to the following:

1. Transform an object.
2. Save it to Cloud Storage using a writable signed URL that you created by using the [`OBJ.GET_ACCESS_URL` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/objectref_functions#objget_access_url).
3. Create an `ObjectRef` value for the transformation output by using the `OBJ.MAKE_REF` function.
4. Save the `ObjectRef` value by writing it to a table column.

**Definitions**

- `uri`: A `STRING` value that contains the URI for the Cloud Storage object, for example, `gs://mybucket/flowers/12345.jpg`. You can also specify a column name in place of a string literal. For example, if you have URI data in a `uri` field, you can specify `OBJ.MAKE_REF(uri, "myproject.us.conn")`.
- `authorizer`: A `STRING` value that contains the [Cloud Resource connection](https://docs.cloud.google.com/bigquery/docs/create-cloud-resource-connection) used for [delegated access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#delegated-access) to the Cloud Storage object. Your data administrator needs to set up the permissions to use this connection. If omitted, the returned ObjectRef uses [direct access](https://docs.cloud.google.com/bigquery/docs/work-with-objectref#direct-access).
- `version_value`: A `STRING` value that represents the Cloud Storage object version.
- `gcs_metadata_json`: A `JSON` value that represents Cloud Storage metadata,
  using the following schema:

      gcs_metadata JSON {
          "content_type": string,
          "md5_hash": string,
          "size": number,
          "updated": number
      }

- `objectref_json`: A `JSON` value that represents a Cloud Storage object,
  using the following schema:

      obj_ref json {
        uri string,
        [, authorizer string ]
        [, version string]
        [, details gcs_metadata_json ]
      }

Validation is performed on the formatting of the input, but not the content.

**Output**

An `ObjectRef` value.

- If you provide a URI as input, then the output is a reference to the Cloud Storage object identified by the URI.
- If you provide an ObjectRef JSON value, then the output contains all of the input information formatted as an ObjectRef value.
- If you provide an ObjectRef value and authorizer, then the output contains the input ObjectRef value with an updated authorizer.

**Examples**

The following example creates an `ObjectRef` value using a URI and a Cloud
resource connection as input:

    CREATE OR REPLACE TABLE `mydataset.movies` AS (
      SELECT
        f.title,
        f.director
        OBJ.MAKE_REF(p.uri, 'asia-south2.storage_connection') AS movie_poster
      FROM mydataset.movie_posters p
      join mydataset.films f
      using(title)
      where region = 'US'
      and release_year = 2024
    );

The following example creates an `ObjectRef` value using JSON input:

    OBJ.MAKE_REF(JSON '{"uri": "gs://cloud-samples-data/bigquery/tutorials/cymbal-pets/images/aquaclear-aquarium-background-poster.png", "authorizer": "asia-south2.storage_connection"}');

The following example creates a new `ObjectRef` value with an updated
authorizer:

    SELECT
      OBJ.MAKE_REF(movie_poster,
                   authorizer=>'asia-south2.new_connection') AS movie_poster_updated
    FROM mydataset.movies

**Limitations**

You can't have more than 20 Cloud resource connections in the project and
region where your query accesses object data as `ObjectRef` values.