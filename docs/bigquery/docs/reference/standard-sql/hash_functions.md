GoogleSQL for BigQuery supports the following hash functions.

## Function list

| Name | Summary |
|---|---|
| [`FARM_FINGERPRINT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#farm_fingerprint) | Computes the fingerprint of a `STRING` or `BYTES` value, using the FarmHash Fingerprint64 algorithm. |
| [`MD5`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#md5) | Computes the hash of a `STRING` or `BYTES` value, using the MD5 algorithm. |
| [`SHA1`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha1) | Computes the hash of a `STRING` or `BYTES` value, using the SHA-1 algorithm. |
| [`SHA256`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha256) | Computes the hash of a `STRING` or `BYTES` value, using the SHA-256 algorithm. |
| [`SHA512`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/hash_functions#sha512) | Computes the hash of a `STRING` or `BYTES` value, using the SHA-512 algorithm. |

## `FARM_FINGERPRINT`

    FARM_FINGERPRINT(value)

**Description**

Computes the fingerprint of the `STRING` or `BYTES` input using the
`Fingerprint64` function from the
[open-source FarmHash library](https://github.com/google/farmhash). The output
of this function for a particular input will never change.

**Return type**

INT64

**Examples**

    WITH example AS (
      SELECT 1 AS x, "foo" AS y, true AS z UNION ALL
      SELECT 2 AS x, "apple" AS y, false AS z UNION ALL
      SELECT 3 AS x, "" AS y, true AS z
    )
    SELECT
      *,
      FARM_FINGERPRINT(CONCAT(CAST(x AS STRING), y, CAST(z AS STRING)))
        AS row_fingerprint
    FROM example;
    /*---+---+---+---+
     | x | y     | z     | row_fingerprint      |
     +---+---+---+---+
     | 1 | foo   | true  | -1541654101129638711 |
     | 2 | apple | false | 2794438866806483259  |
     | 3 |       | true  | -4880158226897771312 |
     +---+---+---+---*/

## `MD5`

    MD5(input)

**Description**

Computes the hash of the input using the
[MD5 algorithm](https://en.wikipedia.org/wiki/MD5). The input can either be
`STRING` or `BYTES`. The string version treats the input as an array of bytes.

This function returns 16 bytes.

> [!WARNING]
> **Warning:** MD5 is no longer considered secure. For increased security use another hashing function.

**Return type**

`BYTES`

**Example**

    SELECT MD5("Hello World") as md5;

    -- Note that the result of MD5 is of type BYTES, displayed as a base64-encoded string.
    /*---+
     | md5                      |
     +---+
     | sQqNsWTgdUEFt6mb5y4/5Q== |
     +---*/

## `SHA1`

    SHA1(input)

**Description**

Computes the hash of the input using the
[SHA-1 algorithm](https://en.wikipedia.org/wiki/SHA-1). The input can either be
`STRING` or `BYTES`. The string version treats the input as an array of bytes.

This function returns 20 bytes.

> [!WARNING]
> **Warning:** SHA1 is no longer considered secure. For increased security, use another hashing function.

**Return type**

`BYTES`

**Example**

    SELECT SHA1("Hello World") as sha1;

    -- Note that the result of SHA1 is of type BYTES, displayed as a base64-encoded string.
    /*---+
     | sha1                         |
     +---+
     | Ck1VqNd45QIvq3AZd8XYQLvEhtA= |
     +---*/

## `SHA256`

    SHA256(input)

**Description**

Computes the hash of the input using the
[SHA-256 algorithm](https://en.wikipedia.org/wiki/SHA-2). The input can either be
`STRING` or `BYTES`. The string version treats the input as an array of bytes.

This function returns 32 bytes.

**Return type**

`BYTES`

**Example**

    SELECT SHA256("Hello World") as sha256;

## `SHA512`

    SHA512(input)

**Description**

Computes the hash of the input using the
[SHA-512 algorithm](https://en.wikipedia.org/wiki/SHA-2). The input can either be
`STRING` or `BYTES`. The string version treats the input as an array of bytes.

This function returns 64 bytes.

**Return type**

`BYTES`

**Example**

    SELECT SHA512("Hello World") as sha512;