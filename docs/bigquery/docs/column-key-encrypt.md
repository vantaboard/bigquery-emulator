# Column-level encryption with Cloud KMS

You can use [Cloud Key Management Service (Cloud KMS)](https://docs.cloud.google.com/kms/docs/concepts)
to encrypt the keys that in turn encrypt the values within
BigQuery tables. You can use the
[AEAD encryption
functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions)
with Cloud KMS [keysets](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#keysets)
or [wrapped keysets](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#wrapped_keysets)
to provide a second layer of protection at the column level.

## Introduction

To provide an extra layer of protection, Cloud KMS encrypts your
data encryption key (DEK) with a second key encryption key (KEK).
In BigQuery, referencing an encrypted keyset instead of a
plaintext keyset helps reduce the risk of key exposure. The KEK is a symmetric
encryption keyset that is stored securely in Cloud KMS and managed
using Identity and Access Management (IAM) roles and permissions.

BigQuery supports deterministic and non-deterministic encryption
functions. With deterministic encryption, if both the pieces of stored data and
the additional authenticated data (optional) are identical, then the ciphertext
is identical. This allows for support of aggregation and joins
based on the encrypted column. With non-deterministic encryption, the stored
ciphertext is unique regardless of the encrypted data, which prevents
clustering, aggregation, and joins.

At query execution time, you provide the Cloud KMS resource path of the
KEK and the ciphertext from the wrapped DEK. BigQuery calls
Cloud KMS to unwrap the DEK, and then uses that key to decrypt the data
in your query. The non-wrapped version of the DEK is only stored in memory for
the duration of the query, and then destroyed.

If you use Cloud KMS in a [region](https://docs.cloud.google.com/kms/docs/ekm#regions) where
[Cloud External Key Manager](https://docs.cloud.google.com/kms/docs/ekm) is supported, you can use Cloud EKM based keys in
Cloud KMS.

> [!NOTE]
> **Note:** Using Cloud EKM keys might introduce additional latency to each key access.

### Use cases

Use cases for encryption with Cloud KMS keys include the following:

- Externally encrypted data that needs to be stored in BigQuery without storing the keyset in plaintext. Your data can then be exported from the table or decrypted with a SQL query.
- "Double access control" on encrypted data in BigQuery. A user must be granted permission to both the table and the encryption key to read data in cleartext.

| **User Permission Matrix** |||
|   | **Permission on Table** | **No Permission on Table** |
|---|---|---|
| **Permissions on Key** | Read and decrypt encrypted data. | No access. |
| **No Permissions on Key** | Read encrypted data. | No access. |

If a user has permission to access the KMS key and has access to the wrapped
keyset, SQL functions can unwrap the keyset and decrypt the ciphertext. Users
can also use the Cloud KMS
[REST API](https://docs.cloud.google.com/kms/docs/reference/rest)
or
[CLI](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption)
to unwrap the keyset.  

The following query sample uses KMS SQL functions to decrypt non-deterministic
ciphertext:

```googlesql
SELECT
  AEAD.DECRYPT_STRING(
    KEYS.KEYSET_CHAIN(@kms_resource_name, @first_level_keyset),
    ciphertext,
    additional_authenticated_data)
FROM
  ciphertext_table
WHERE
  ...
```

#### Use-case example

Assume an implementation where zip codes are considered
sensitive information. Zip code data can be inserted into the
BigQuery table using the AEAD encrypt function, thereby
encrypting the `Zipcode` column. In this example, we use the
`AEAD.ENCRYPT` function with the wrapped keyset management function. The
`KEYS.KEYSET_CHAIN` function decrypts the digital encryption key with the KEK,
and the `AEAD.ENCRYPT` function passes the information to KMS.

The keyset chain for encryption and decryption ensures that the data encryption
key (DEK) is encrypted or wrapped with a KEK and passed with that KEK. The
wrapped DEK is decrypted or unwrapped within the SQL function, and then used to
encrypt or decrypt data.

The AEAD non-deterministic function can decrypt data when it is accessed by
using the function in the query that is being run on the table.

![image](https://docs.cloud.google.com/static/bigquery/images/aead-encrypt.png)

The AEAD deterministic function can decrypt data when it is accessed by
using the function in the query that is being run on the table and supports
aggregation and joins using the encrypted data.

![image](https://docs.cloud.google.com/static/bigquery/images/deterministic-encrypt.png)

### Non-deterministic function syntax

The supported syntax for using non-deterministic functions includes the
following:

```googlesql
AEAD.ENCRYPT(
  KEYS.KEYSET_CHAIN(kms_resource_name, first_level_keyset),
  plaintext,
  additional_authenticated_data)
```

```googlesql
AEAD.DECRYPT_STRING(
  KEYS.KEYSET_CHAIN(kms_resource_name, first_level_keyset),
  ciphertext,
  additional_authenticated_data)
```

```googlesql
AEAD.DECRYPT_BYTES(
  KEYS.KEYSET_CHAIN(kms_resource_name, first_level_keyset),
  ciphertext,
  additional_authenticated_data)
```

See [`AEAD.DECRYPT_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeaddecrypt_bytes),
[`AEAD.ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeadencrypt),
[`AEAD.DECRYPT_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeaddecrypt_string),
and [`KEYS.KEYSET_CHAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_chain)
function syntax.

### Deterministic function syntax

The supported syntax for using deterministic functions includes the
following:

```googlesql
DETERMINISTIC_ENCRYPT(
  KEYS.KEYSET_CHAIN(kms_resource_name, first_level_keyset),
  plaintext,
  additional_data)
```

```googlesql
DETERMINISTIC_DECRYPT_STRING(
  KEYS.KEYSET_CHAIN(kms_resource_name, first_level_keyset),
  ciphertext,
  additional_data)
```

```googlesql
DETERMINISTIC_DECRYPT_BYTES(
  KEYS.KEYSET_CHAIN(kms_resource_name, first_level_keyset),
  ciphertext,
  additional_data)
```

See [`DETERMINISTIC_DECRYPT_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_decrypt_bytes),
[`DETERMINISTIC_ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_encrypt),
[`DETERMINISTIC_DECRYPT_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#deterministic_decrypt_string),
and [`KEYS.KEYSET_CHAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_chain)
function syntax.

## Roles and permissions

For a list of roles for Cloud KMS,
see [Cloud KMS permissions and roles](https://docs.cloud.google.com/kms/docs/reference/permissions-and-roles#predefined).

## Limitations

Encryption with Cloud KMS has the following limitations and
restrictions:

- The Cloud KMS keys are restricted to the same
  [region or multi-region](https://docs.cloud.google.com/kms/docs/locations)
  as the query. Using global Cloud KMS keys is disallowed for
  reliability reasons.

- It is not possible to rotate a wrapped keyset using the `KEYS.ROTATE_KEYSET`
  function.

- The constant parameters in a BigQuery query are visible
  to users in the [diagnostic query plan](https://docs.cloud.google.com/bigquery/docs/query-plan-explanation).
  This factor can affect the `kms_resource_name` and `first_level_keyset`
  parameters of the `KEYSET_CHAIN` function. Keys are never exposed in
  plaintext, and permission to the Cloud KMS key is required to decrypt
  the wrapped keyset. This approach ensures that keys are not exposed through
  the diagnostic query plan unless the user has permission to decrypt the
  keyset.

- Column-level encryption has the following limitations when used with
  type-based security classifications:

  - Column-level security: Users can only decrypt or encrypt data on columns
    that they are allowed to access.

  - Row-level security: Users can only decrypt data on rows that they are
    allowed to access.

- Column-level SQL functions have no significant impact on performance when
  compared to the performance of raw encryption functions where the key data is
  sent in plaintext.

## Before you begin

To work with Cloud KMS keys, keysets, encrypted tables, deterministic,
and non-deterministic functions, you need to do the following if you have not
already done so:

1. [Create a Google Cloud project](https://docs.cloud.google.com/resource-manager/docs/creating-managing-projects#creating_a_project).

2. [Create a BigQuery dataset](https://docs.cloud.google.com/bigquery/docs/datasets#create-dataset).

3. [Create a Cloud KMS key ring](https://docs.cloud.google.com/kms/docs/creating-keys#create_a_key_ring).

4. [Create a Cloud KMS key](https://docs.cloud.google.com/kms/docs/creating-keys#create_a_key)
   for an encrypted column with the software or
   [Hardware Security Module (HSM)](https://docs.cloud.google.com/kms/docs/hsm) protection level.

5. [Grant user permissions to work with Cloud KMS keys, encryption,
   and decryption](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#roles).

Take note of the following concepts, as they are referenced in the next
sections:

- `PROJECT_ID`: The name of the Google Cloud project.

- `DATASET_NAME`: The name of the BigQuery dataset.

- `LOCATION_ID`: The location of the BigQuery dataset.

- `TABLE_NAME`: The name of the BigQuery table.

- `KEY_RING_ID`: The name of the Cloud KMS key ring.

- `KEY_ID`: The name of the Cloud KMS key.

- `KMS_KEY`: Cloud KMS key (KEK) in this format:

  ```googlesql
  'gcp-kms://projects/PROJECT_ID/locations/LOCATION_ID/keyRings/KEY_RING_ID/cryptoKeys/KEY_ID'
  ```

  Here is an example of a Cloud KMS key:

      'gcp-kms://projects/myProject/locations/us/keyRings/myKeyRing/cryptoKeys/myKeyName'

- `KMS_KEY_SHORT`: Similar to `KMS_KEY` but in this format:

  ```googlesql
  projects/PROJECT_ID/locations/LOCATION_ID/keyRings/KEY_RING_ID/cryptoKeys/KEY_ID
  ```
- `KEYSET_DECODED`: A decoded keyset as a `BYTES` sequence. The output looks
  similar to that for a decoded wrapped keyset.

  Although keyset functions return keysets as bytes, the
  user output is displayed as an encoded string. To convert an encoded
  keyset to a decoded keyset, see
  [Decode a Cloud KMS keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#decode-wrapped-keyset)
- `KEYSET_ENCODED`: An encoded keyset as a `STRING`. The output looks similar
  to that for an encoded wrapped keyset.

  To convert an encoded keyset to a decoded keyset, see
  [Decode a Cloud KMS keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#decode-wrapped-keyset)
- `WRAPPED_KEYSET_DECODED`: A decoded wrapped keyset as a `BYTES` sequence.
  Here is an example of what the output for this looks like:

      b'\x0a$\x00\xa6\xee\x12Y\x8d|l"\xf7\xfa\xc6\xeafM\xdeefy\xe9\x7f\xf2z\xb3M\
      xf6"\xd0\xe0Le\xa8\x8e\x0fR\xed\x12\xb7\x01\x00\xf0\xa80\xbd\xc1\x07Z\\
      \xd0L<\x80A0\x9ae\xfd(9\x1e\xfa\xc8\x93\xc7\xe8\...'

  Although wrapped keyset functions return wrapped keysets as bytes, the
  user output is displayed as an encoded string. To convert an encoded
  wrapped keyset to a decoded wrapped keyset, see
  [Decode a Cloud KMS keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#decode-wrapped-keyset)
- `WRAPPED_KEYSET_ENCODED`: An encoded wrapped keyset as a `STRING`.
  Here is an example of what the output for this looks like:

      'CiQApu4SWTozQ7lNwITxpEvGlo5sT2rv1tyuSv3UAMtoTq/lhDwStwEA8KgwvX7CpVVzhWWMkRw
      WZNr3pf8uBIlzHeunCy8ZsQ6CofQYFpiBRBB6k/QqATbiFV+3opnDk/6dBL/S8OO1WoDC+DdD9
      uzEFwqt5D20lTXCkGWFv1...'

  To convert an encoded wrapped keyset to a decoded wrapped keyset, see
  [Decode a Cloud KMS keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#decode-wrapped-keyset)

## Key management

The following sections contain common tasks you can perform with
Cloud KMS keys.

### Create a keyset

You can create wrapped keysets or raw keysets. To do this, complete the steps
in the following sections.

#### Create a raw keyset

Run the following query to create a keyset with a key of type
`DETERMINISTIC_AEAD_AES_SIV_CMAC_256`.

```googlesql
SELECT KEYS.NEW_KEYSET('DETERMINISTIC_AEAD_AES_SIV_CMAC_256') AS raw_keyset
```

#### Create a wrapped keyset

Run the following query to create a Cloud KMS wrapped keyset with a
key of type `DETERMINISTIC_AEAD_AES_SIV_CMAC_256`.

```googlesql
SELECT KEYS.NEW_WRAPPED_KEYSET(
  KMS_KEY,
  'DETERMINISTIC_AEAD_AES_SIV_CMAC_256')
```

### Decode a keyset

Although SQL functions that return keysets produce the keysets in
`BYTES` format, the user-displayed result is encoded and displayed in
`STRING` format. If you would like to convert this encoded string to a
decoded bytes sequence that you can use as literal key encryption functions,
use the following query.

#### Decode a wrapped keyset

Run the following query to decode a Cloud KMS wrapped keyset.

```googlesql
SELECT FORMAT('%T', FROM_BASE64(WRAPPED_KEYSET_ENCODED'))
```

#### Decode a raw keyset

Run the following query to decode a raw keyset.

```googlesql
SELECT FORMAT('%T', FROM_BASE64(KEYSET_ENCODED'))
```

### Rewrap a wrapped keyset

Run the following query to rewrap a Cloud KMS wrapped keyset with a new
Cloud KMS key. `KMS_KEY_CURRENT` represents the new `KMS_KEY` that is
used to encrypt the keyset. `KMS_KEY_NEW` represents the new `KMS_KEY` that is
used to encrypt the keyset.

```googlesql
SELECT KEYS.REWRAP_KEYSET(
  KMS_KEY_CURRENT,
  KMS_KEY_NEW,
  WRAPPED_KEYSET_DECODED)
```

### Rotate a wrapped keyset

Run the following query to rotate a Cloud KMS wrapped keyset with a
key of type `DETERMINISTIC_AEAD_AES_SIV_CMAC_256`.

```googlesql
SELECT KEYS.ROTATE_WRAPPED_KEYSET(
  KMS_KEY,
  WRAPPED_KEYSET_DECODED,
  'DETERMINISTIC_AEAD_AES_SIV_CMAC_256')
```

### Generate a raw keyset from a wrapped keyset

Some encryption functions require a raw keyset. To decrypt a
Cloud KMS wrapped keyset to produce a raw keyset, complete the
following steps.

1. [Create a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#wrap-keyset).

2. In the bq command-line tool enter the following commands to save a wrapped keyset in
   a file called `keyset_to_unwrap`, decrypt the wrapped keyset, and
   produce the output in the `KEYSET_DECODED` format:

   ```bash
   echo WRAPPED_KEYSET_ENCODED | base64 -d > /tmp/decoded_wrapped_key
   ```

   ```bash
   gcloud kms decrypt \
   --ciphertext-file=/tmp/decoded_wrapped_key \
   --key=KMS_KEY_SHORT \
   --plaintext-file=/tmp/keyset_to_unwrap.dec \
   --project=PROJECT_ID
   ```

   ```bash
   od -An --format=o1 /tmp/keyset_to_unwrap.dec | tr ' ' '\'
   ```

### Generate a wrapped keyset from a raw keyset

Some encryption functions require a Cloud KMS wrapped keyset. To
encrypt a raw keyset to produce a wrapped keyset, complete the following steps.

1. [Create a raw keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#raw-keyset).

2. In the bq command-line tool enter the following commands to save a raw keyset in
   a file called `keyset_to_wrap`, encrypt the raw keyset, and
   produce the output in the `WRAPPED_KEYSET_DECODED` format:

   ```bash
   echo KEYSET_ENCODED | base64 -d > /tmp/decoded_key
   ```

   ```bash
   gcloud kms encrypt \
   --plaintext-file=/tmp/decoded_key \
   --key=KMS_KEY_SHORT \
   --ciphertext-file=/tmp/keyset_to_wrap.dec \
   --project=PROJECT_ID
   ```

   ```bash
   od -An --format=o1 /tmp/keyset_to_wrap.dec | tr ' ' '\'
   ```

### Generate a wrapped key for a DLP function

For [DLP functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dlp_functions), you
need a cryptographic key and then use that key to get a wrapped key.

1. To generate a new cryptographic key, on the [command line](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool),
   run the following command. The size of the key can be 16, 24, or 32 bytes.
   The following example uses a 16-byte key:

       openssl rand 16 > rand.key.16.bin

2. Wrap the generated 16-byte key with a [KMS key](https://docs.cloud.google.com/sdk/gcloud/reference/kms/encrypt).
   See the following example:

       KEYRING=projects/myproject/locations/us/keyRings/kms-test
       KEY=projects/myproject/locations/us/keyRings/kms-test/cryptoKeys/test-Kek
       PROJECT="myproject"

       gcloud kms encrypt --project $PROJECT --location us --keyring $KEYRING --key $KEY --plaintext-file ./rand.key.16.bin --ciphertext-file ./rand.key.16.wrapped

3. You can now get the `BYTES` literal of the wrapped key or the base64 format
   of the wrapped key.

   - **Bytes literal**

         username:~/tmp$ od -b ./rand.key.16.wrapped | cut -d ' ' -f 2- | head -n -1 | sed  -e 's/^/ /' | tr ' ' '\'

     The output looks like the following:

         \012\044\000\325\155\264\153\246\071\172\130\372\305\103\047\342\356\061\077\014\030\126\147\041\126\150\012\036\020\202\215\044\267\310\331\014\116\233\022\071\000\363\344\230\067\274\007\340\273\016\212\151\226\064\200\377\303\207\103\147\052\267\035\350\004\147\365\251\271\133\062\251\246\152\177\017\005\270\044\141\211\116\337\043\035\263\122\340\110\333\266\220\377\247\204\215\233

   - **Base64 format**

         username:~/tmp$ base64 ./rand.key.16.wrapped

     The output looks like the following:

         CiQA1W20a6Y5elj6xUMn4u4xPwwYVmchVmgKHhCCjSS3yNkMTpsSOQDz5Jg3vAfguw6KaZY0gP/Dh0NnKrcd6ARn9am5WzKppmp/DwW4JGGJTt8jHbNS4EjbtpD/p4SNmw==

### Get the number of keys in a keyset

Run the following query to get the number of keys in a
raw keyset.

1. If you are working with a wrapped keyset, first
   [generate a raw keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#generate-raw-keyset).

2. Run this query with the raw keyset:

   ```googlesql
   SELECT KEYS.KEYSET_LENGTH(KEYSET_DECODED) as key_count;
   ```

### Get a JSON representation of a keyset

Run the following query to view a JSON representation of a
raw keyset.

1. If you are working with a wrapped keyset, first
   [generate a raw keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#generate-raw-keyset).

2. Run this query with the raw keyset:

   ```googlesql
   SELECT KEYS.KEYSET_TO_JSON(KEYSET_DECODED);
   ```

## Encryption and decryption

You can use raw keysets or wrapped keysets to encrypt a column in a table. You
can also choose to use deterministic or non-deterministic encryption on your
columns. The examples in this section use wrapped keysets but you could replace
the wrapped keysets with raw keysets.

### Deterministically encrypt a column with a wrapped keyset

Run the following query to create a table and store a
Cloud KMS wrapped keyset with deterministic encryption in a column
called `encrypted_content`.

1. [Create a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#wrap-keyset).

2. Encrypt a column with the wrapped keyset.

   ```googlesql
   CREATE OR REPLACE TABLE DATASET_NAME.TABLE_NAME AS
     SELECT DETERMINISTIC_ENCRYPT(
       KEYS.KEYSET_CHAIN(KMS_KEY, WRAPPED_KEYSET_DECODED),
       'plaintext',
       '') AS encrypted_content
   ```

### Deterministically decrypt a column with a wrapped keyset

Run the following query to deterministically decrypt a column that
contains encrypted content, using a Cloud KMS wrapped keyset. This
query assumes you are referencing a table with a column called
`encrypted_content`.

```googlesql
SELECT DETERMINISTIC_DECRYPT_STRING(
  KEYS.KEYSET_CHAIN(KMS_KEY, WRAPPED_KEYSET_DECODED),
  encrypted_content,
  '')
FROM DATASET_NAME.TABLE_NAME
```

### Non-deterministically encrypt a column with a wrapped keyset

See [Deterministically encrypt a column with a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#determine-encrypt-column),
but replace `DETERMINISTIC_ENCRYPT` with `AEAD.ENCRYPT`. Make sure that your
keyset is of type `AEAD_AES_GCM_256`.

### Non-deterministically decrypt a column with a wrapped keyset

See [Deterministically decrypt a column with a wrapped keyset](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt#determine-decrypt-column),
but replace `DETERMINISTIC_DECRYPT_STRING` with `AEAD.DECRYPT_STRING`. Make sure
that your keyset is of type `AEAD_AES_GCM_256`.

## What's next

- Learn more about [Cloud KMS](https://docs.cloud.google.com/kms/docs/resource-hierarchy). This topic includes conceptual information about column-level encryption for Google Cloud.
- Learn more about [AEAD encryption for BigQuery](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts). This topic includes conceptual information about column-level encryption specifically for BigQuery.
- Learn more about [AEAD encryption functions for BigQuery](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions). This topic contains all of the SQL functions that you can use for column-level encryption in BigQuery.