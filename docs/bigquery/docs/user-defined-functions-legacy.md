# User-defined functions in legacy SQL


This document details how to use JavaScript user-defined functions in legacy SQL query syntax.
The preferred query syntax for user-defined functions in BigQuery is [GoogleSQL syntax](https://docs.cloud.google.com/bigquery/docs/user-defined-functions).
For more information, see [Legacy SQL feature availability](https://docs.cloud.google.com/bigquery/docs/legacy-sql-feature-availability).

BigQuery legacy SQL supports user-defined functions (UDFs) written in JavaScript.
A UDF is similar to the "Map" function in a MapReduce: it takes a single row as input and produces
zero or more rows as output. The output can potentially have a different schema than the input.

For information on user-defined functions in GoogleSQL, see
[User-defined functions in GoogleSQL](https://docs.cloud.google.com/bigquery/sql-reference/user-defined-functions).

## UDF example

```bash
// UDF definition
function urlDecode(row, emit) {
  emit({title: decodeHelper(row.title),
        requests: row.num_requests});
}

// Helper function with error handling
function decodeHelper(s) {
  try {
    return decodeURI(s);
  } catch (ex) {
    return s;
  }
}

// UDF registration
bigquery.defineFunction(
  'urlDecode',  // Name used to call the function from SQL

  ['title', 'num_requests'],  // Input column names

  // JSON representation of the output schema
  [{name: 'title', type: 'string'},
   {name: 'requests', type: 'integer'}],

  urlDecode  // The function reference
);
```

[Back to top](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#top)

## UDF structure

```bash
function name(row, emit) {
  emit(<output data>);
}
```

BigQuery UDFs operate on individual rows of a table or subselect query results. The
UDF has two formal parameters:

- `row`: an input row.
- `emit`: a hook used by BigQuery to collect output data. The `emit` function takes one parameter: a JavaScript object that represents a single row of output data. The `emit` function can be called more than once, such as in a loop, to output multiple rows of data.

The following code example shows a basic UDF.

```bash
function urlDecode(row, emit) {
  emit({title: decodeURI(row.title),
        requests: row.num_requests});
}
```

### Registering the UDF

You must register a name for your function so that it can be invoked from BigQuery
SQL. The registered name doesn't have to match the name you used for your function in JavaScript.

```bash
bigquery.defineFunction(
  '\<UDF name\>',  // Name used to call the function from SQL

  ['\<col1\>', '\<col2\>'],  // Input column names

  // JSON representation of the output schema
  [<output schema>],

  // UDF definition or reference
  <UDF definition or reference>
);
```

#### Input columns

The input column names must match the names (or aliases, if applicable) of the columns in the
input table or subquery.

For input columns that are records, you must specify---in the input column list---the
leaf fields that you want to access from the record.

For example, if you have a record that stores a person's name and age:

```
person RECORD REPEATED
  name STRING OPTIONAL
  age INTEGER OPTIONAL
```

The input specifier for the name and age would be:

```
['person.name', 'person.age']
```

Use of `['person']` without the name or age would generate an error.

The resulting output will match the schema; you'll have an array of JavaScript objects, where
each object has a "name" and an "age" property. For example:

```
[ {name: 'alice', age: 23}, {name: 'bob', age: 64}, ... ]
```

#### Output schema

You must provide BigQuery with the schema or structure of the records your UDF
produces, represented as JSON. The schema can contain
[any supported BigQuery data types](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-types),
including nested records. The supported type specifiers are:

- boolean
- float
- integer
- record
- string
- timestamp

The following code example shows the syntax for records in the output schema. Each output field
requires a `name` and `type` attribute. Nested fields must also contain a
`fields` attribute.

```json
[{name: 'foo_bar', type: 'record', fields:
  [{name: 'a', type: 'string'},
   {name: 'b', type: 'integer'},
   {name: 'c', type: 'boolean'}]
}]
```

Each field can contain an optional `mode` attribute, which supports the following values:

- nullable : this is the default and may be omitted.
- required : if specified, the given field must be set to a value and cannot be undefined.
- repeated : if specified, the given field must be an array.

Rows passed to the `emit()` function must match the data types of the output schema.
Fields represented in the output schema that are omitted in the emit function will output as nulls.

#### UDF definition or reference

If you prefer, you can define the UDF inline in `bigquery.defineFunction`. For example:

```bash
bigquery.defineFunction(
  'urlDecode',  // Name used to call the function from SQL

  ['title', 'num_requests'],  // Input column names

  // JSON representation of the output schema
  [{name: 'title', type: 'string'},
   {name: 'requests', type: 'integer'}],

  // The UDF
  function(row, emit) {
    emit({title: decodeURI(row.title),
          requests: row.num_requests});
  }
);
```

Otherwise, you can define the UDF separately, and pass a reference to the function in
`bigquery.defineFunction`. For example:

```bash
// The UDF
function urlDecode(row, emit) {
  emit({title: decodeURI(row.title),
        requests: row.num_requests});
}

// UDF registration
bigquery.defineFunction(
  'urlDecode',  // Name used to call the function from SQL

  ['title', 'num_requests'],  // Input column names

  // JSON representation of the output schema
  [{name: 'title', type: 'string'},
   {name: 'requests', type: 'integer'}],

  urlDecode  // The function reference
);
```

#### Error handling

If an exception or error is thrown during the processing of a UDF, the entire query will fail.
You can use a try-catch block to handle errors. For example:

```bash
// The UDF
function urlDecode(row, emit) {
  emit({title: decodeHelper(row.title),
        requests: row.num_requests});
}

// Helper function with error handling
function decodeHelper(s) {
  try {
    return decodeURI(s);
  } catch (ex) {
    return s;
  }
}

// UDF registration
bigquery.defineFunction(
  'urlDecode',  // Name used to call the function from SQL

  ['title', 'num_requests'],  // Input column names

  // JSON representation of the output schema
  [{name: 'title', type: 'string'},
   {name: 'requests', type: 'integer'}],

  urlDecode  // The function reference
);
```

## Running a query with a UDF

You can use UDFs in legacy SQL with the [bq command-line tool](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#command-line)
or the [BigQuery API](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#api). The
[Google Cloud console](https://docs.cloud.google.com/bigquery/docs/bigquery-web-ui) doesn't
support UDFs in legacy SQL.

### Using the bq command-line tool

To run a query containing one or more UDFs, specify the `--udf_resource`
flag in the bq command-line tool from the Google Cloud CLI. The value of the flag can be
either a Cloud Storage (`gs://...`) URI or the path to a local
file. To specify multiple UDF resource files, repeat this flag.

Use the following syntax to run a query with a UDF:

```
bq query --udf_resource=<file_path_or_URI> <sql_query>
```

The following example runs a query that uses a UDF stored in a local file and a SQL query
that is also stored in a local file.

#### Creating the UDF

You can store the UDF in Cloud Storage or as a local text file. For
example, to store the following `urlDecode` UDF, create a file
named `urldecode.js` and paste the following JavaScript code into
the file before saving the file.

```
// UDF definition
function urlDecode(row, emit) {
  emit({title: decodeHelper(row.title),
        requests: row.num_requests});
}

// Helper function with error handling
function decodeHelper(s) {
  try {
    return decodeURI(s);
  } catch (ex) {
    return s;
  }
}

// UDF registration
bigquery.defineFunction(
  'urlDecode',  // Name used to call the function from SQL

  ['title', 'num_requests'],  // Input column names

  // JSON representation of the output schema
  [{name: 'title', type: 'string'},
   {name: 'requests', type: 'integer'}],

  urlDecode  // The function reference
);
```

#### Creating the query

You can also store the query in a file to keep your command line
from becoming too verbose. For example, you can create a local
file named `query.sql` and paste the following BigQuery
statement into the file.

```googlesql
#legacySQL
SELECT requests, title
FROM
  urlDecode(
    SELECT
      title, sum(requests) AS num_requests
    FROM
      [my-project:wikipedia.pagecounts_201504]
    WHERE language = 'fr'
    GROUP EACH BY title
  )
WHERE title LIKE '%ç%'
ORDER BY requests DESC
LIMIT 100
```

After saving the file you can reference the file on the command line.

#### Running the query

After defining the UDF and the query in separate files,
you can reference them in the command line.
For example, the following command runs the query that
you saved as the file named `query.sql` and references the UDF that you created.

```
$ bq query --udf_resource=urldecode.js "$(cat query.sql)"
```

### Using the BigQuery API

#### configuration.query

Queries that use UDFs must contain
[`userDefinedFunctionResources`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#userdefinedfunctionresource)
elements that provide the code, or locations to code resources, to be used in the query. The
supplied code must include registration function invocations for any UDFs referenced by the query.

#### Code resources

Your query configuration may include JavaScript code blobs, as well as references to JavaScript
source files stored in Cloud Storage.

Inline JavaScript code blobs are populated in the
[`inlineCode`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#UserDefinedFunctionResource.FIELDS.inline_code)
section of a `userDefinedFunctionResource` element. However, code that will be reused
or referenced across multiple queries should be persisted in Cloud Storage and referenced as
an external resource.

To reference a JavaScript source file from Cloud Storage, set the
[`resourceURI`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables#UserDefinedFunctionResource.FIELDS.resource_uri)
section of the `userDefinedFunctionResource` element to the file's `gs://`
URI.

The query configuration may contain multiple `userDefinedFunctionResource` elements.
Each element may contain either an `inlineCode` or a `resourceUri` section.

#### Example

The following JSON example illustrates a query request that references two UDF resources: one
blob of inline code, and one file `lib.js` to be read from Cloud Storage. In this
example, `myFunc` and the registration invocation for `myFunc` are provided
by `lib.js`.

```bash
{
  "configuration": {
    "query": {
      "userDefinedFunctionResources": [
        {
          "inlineCode": "var someCode = 'here';"
        },
        {
          "resourceUri": "gs://some-bucket/js/lib.js"
        }
      ],
      "query": "select a from myFunc(T);"
    }
  }
}
```

[Back to top](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#top)

## Best practices

#### Developing your UDF

You can use [our UDF
test tool](https://github.com/GoogleCloudPlatform/bigquery-udf-test-tool) to test and debug your UDF without running up your BigQuery bill.

#### Pre-filter your input

If your input can be easily filtered down before being passed to a UDF, your query will likely be
faster and cheaper.

In the [running a query](https://docs.cloud.google.com/bigquery/user-defined-functions#queryui) example, a subquery
is passed as the input to `urlDecode`, instead of a full table. A table might have
billions of rows, and if we ran the UDF on the entire table, the JavaScript framework would need
to process many more rows than it would with the filtered subquery.

#### Avoid persistent mutable state

Do not store or access mutable state across UDF calls. The following code example describes this scenario:

```bash
// myCode.js
var numRows = 0;

function dontDoThis(r, emit) {
  emit({rowCount: ++numRows});
}

// The query.
SELECT max(rowCount) FROM dontDoThis(t);
```

The above example will not behave as expected, because BigQuery shards your query
across many nodes. Each node has a standalone JavaScript processing environment that accumulates
separate values for `numRows`.

#### Use memory efficiently

The JavaScript processing environment has limited memory available per query. UDF queries that
accumulate too much local state may fail due to memory exhaustion.

#### Expand select queries

You must explicitly list the columns being selected from a UDF.
`SELECT * FROM \<UDF name\>(...)` isn't supported.

To examine the structure of the input row data, you can use `JSON.stringify()` to emit
a string output column:

```bash
bigquery.defineFunction(
  'examineInputFormat',
  ['some', 'input', 'columns'],
  [{name: 'input', type: 'string'}],
  function(r, emit) {
    emit({input: JSON.stringify(r)});
  }
);
```

[Back to top](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#top)

## Limits

- The amount of data that your UDF outputs when processing a single row should be approximately 5 MB or less.
- Each user is limited to running approximately 6 UDF queries in a specific project at the same time. If you receive an error that you're over the [concurrent query limit](https://docs.cloud.google.com/bigquery/quota-policy#queries), wait a few minutes and try again.
- A UDF can timeout and prevent your query from completing. Timeouts can be as short as 5 minutes, but can vary depending on several factors, including how much user CPU time your function consumes and how large your inputs and outputs to the JS function are.
- A query job can have a maximum of 50 UDF resources (inline code blobs or external files).
- Each inline code blob is limited to a maximum size of 32 KB. To use larger code resources, store your code in Cloud Storage and reference it as an external resource.
- Each external code resource is limited to a maximum size of 1 MB.
- The cumulative size of all external code resources is limited to a maximum of 5 MB.

[Back to top](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#top)

## Limitations

- The DOM objects `Window`, `Document` and `Node`, and functions that require them, are unsupported.
- JavaScript functions that rely on native code are unsupported.
- Bitwise operations in JavaScript handle only the most significant 32 bits.
- Because of their non-deterministic nature, queries that invoke user-defined functions cannot use cached results.

[Back to top](https://docs.cloud.google.com/bigquery/docs/user-defined-functions-legacy#top)