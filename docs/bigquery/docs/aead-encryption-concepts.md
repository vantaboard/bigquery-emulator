GoogleSQL for BigQuery supports Authenticated Encryption with Associated Data (AEAD)
encryption.

This topic explains the concepts behind AEAD encryption in GoogleSQL.
For a description of the different AEAD encryption functions that
GoogleSQL supports, see
[AEAD encryption functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions).

### Purpose of AEAD encryption

BigQuery keeps your data safe by using
encryption at rest. BigQuery also provides support for customer managed
encryption keys (CMEKs), which lets you encrypt tables using specific encryption
keys. In some cases, however, you may want to encrypt individual values within a
table.

For example, you want to keep data for all of your own customers in a common
table, and encrypt each of your customers' data using a different key. You have
data spread across multiple tables that you want to be able to
"crypto-delete". Crypto-deletion, or crypto-shredding, is the process of
deleting an encryption key to render unreadable any data encrypted using that
key.

AEAD encryption functions allow you to create keysets that contain keys for
encryption and decryption, use these keys to encrypt and decrypt individual
values in a table, and rotate keys within a keyset.

### Keysets

A keyset is a collection of cryptographic keys, one of which is the primary
cryptographic key and the rest of which, if any, are secondary cryptographic
keys. Each key encodes an
[algorithm for encryption or decryption](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#block_cipher_modes); whether the key
is enabled, disabled, or destroyed; and, for non-destroyed keys, the key bytes
themselves. The primary cryptographic key determines how to encrypt input
plaintext. The primary cryptographic key can never be in a disabled state.
Secondary cryptographic keys are only for decryption and can be either in an
enabled or disabled state. A keyset can be used to decrypt any data that it was
used to encrypt.

The representation of a keyset in GoogleSQL is as a serialized
[google.crypto.tink.Keyset](https://github.com/google/tink/blob/master/proto/tink.proto)
protocol buffer in `BYTES`.

**Example**

The following is an example of an AEAD keyset, represented as a JSON string,
with three keys.

    {
      "primaryKeyId": 569259624,
      "key": [
        {
          "keyData": {
            "typeUrl": "type.googleapis.com/google.crypto.tink.AesGcmKey",
            "value": "GiDPhTp5gIhfnDb6jfKOT4SmNoriIJc7ah8uRvrCpdNihA==",
            "keyMaterialType": "SYMMETRIC"
          },
          "status": "ENABLED",
          "keyId": 569259624,
          "outputPrefixType": "TINK"
        },
        {
          "keyData": {
            "typeUrl": "type.googleapis.com/google.crypto.tink.AesGcmKey",
            "value": "GiBp6aU2cFbVfTh9dTQ1F0fqM+sGHXc56RDPryjAnzTe2A==",
            "keyMaterialType": "SYMMETRIC"
          },
          "status": "DISABLED",
          "keyId": 852264701,
          "outputPrefixType": "TINK"
        },
        {
          "status": "DESTROYED",
          "keyId": 237910588,
          "outputPrefixType": "TINK"
        }
      ]
    }

In the above example, the primary cryptographic key has an ID of `569259624` and
is the first key listed in the JSON string. There are two secondary
cryptographic keys, one with ID `852264701` in a disabled state, and another
with ID `237910588` in a destroyed state. When an AEAD encryption function uses
this keyset for encryption, the resulting ciphertext encodes the primary
cryptographic key's ID of `569259624`.

When an AEAD function uses this keyset for decryption, the function chooses the
appropriate key for decryption based on the key ID encoded in the ciphertext; in
the example above, attempting to decrypt using either key IDs `852264701` or
`237910588` would result in an error, because key ID `852264701` is disabled and
ID `237910588` is destroyed. Restoring key ID `852264701` to an enabled state
would render it usable for decryption.

The key type determines the [encryption mode](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#block_cipher_modes) to use with
that key.

Encrypting plaintext more than once using the same keyset generally returns
different ciphertext values due to different
[initialization vectors (IVs)](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#block_cipher_modes), which are chosen using the
pseudo-random number generator provided by OpenSSL.

> [!NOTE]
> **Note:** If you attempt to pass keysets in plaintext as part of queries, the query text may be logged, and with them the plaintext keyset. You can use BigQuery's [parameterized queries](https://cloud.google.com/bigquery/docs/parameterized-queries) to avoid logging the plaintext keyset.

### Wrapped keysets

If you need to securely manage a keyset or transmit it over an
untrusted channel, consider using a wrapped keyset. When you wrap a
raw keyset, this process encrypts the raw keyset using a
[Cloud KMS key](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#cloud_kms_protection).

Wrapped keysets can encrypt and decrypt data without exposing the keyset data.
While there might be other ways to restrict access to field-level data, wrapped
keysets provide a more secure mechanism for keyset management compared to
raw keysets.

As with [keysets](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#keysets), wrapped keysets can, and should, be periodically
rotated. Wrapped keysets are used in
[AEAD envelope encryption functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions).

Here are some functions with wrapped keyset examples:

- [`KEYS.NEW_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysnew_wrapped_keyset): Create a new wrapped keyset.
- [`KEYS.ROTATE_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrotate_wrapped_keyset): Rotate a wrapped keyset.
- [`KEYS.REWRAP_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrewrap_keyset): Rewrap a wrapped keyset with new data.
- [`KEYS.KEYSET_CHAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_chain): Get a [Tink](https://github.com/google/tink/blob/master/proto/tink.proto) keyset that is encrypted with a [Cloud KMS key](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#cloud_kms_protection).

### Advanced Encryption Standard (AES)

[AEAD encryption functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions) use
[Advanced Encryption Standard (AES) encryption](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard).
AES encryption takes plaintext as input, along with a cryptographic key, and
returns an encrypted sequence of bytes as output. This
sequence of bytes can later be decrypted using the same key as was used to
encrypt it. AES uses a block size of 16 bytes, meaning that the plaintext is
treated as a sequence of 16-byte blocks. The ciphertext will contain a
Tink-specific prefix indicating the key used to perform the encryption. AES
encryption supports multiple [block cipher modes](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#block_cipher_modes).

### Block cipher modes

Two block cipher modes supported by AEAD encryption functions are GCM and CBC.

#### GCM

[Galois/Counter Mode (GCM)](https://en.wikipedia.org/wiki/Galois%2FCounter_Mode)
is a mode for AES encryption. The function numbers blocks sequentially, and then
combines this block number with an initialization vector (IV). An initialization
vector is a random or pseudo-random value that forms the basis of the
randomization of the plaintext data. Next, the function encrypts the combined
block number and IV using AES. The function then performs a bitwise
logical exclusive or (XOR) operation on the result of the encryption and the
plaintext to produce the ciphertext. GCM mode uses a cryptographic key of
128 or 256 bits in length.

#### CBC mode

CBC "chains" blocks by XORing each block of plaintext with the previous block
of ciphertext prior to encrypting it. CBC mode uses a cryptographic key of
either 128, 192, or 256 bits in length. CBC uses a 16-byte initialization
vector as the initial block and XORs this block with the first plaintext block.

CBC mode isn't an
[AEAD scheme in the cryptographic sense](https://en.wikipedia.org/wiki/Authenticated_encryption)
as it doesn't provide data integrity; in other words, malicious modifications
to the encrypted data will not be detected, which compromises data
confidentiality as well. CBC is therefore not recommended unless necessary for
legacy reasons.

### Additional data

AEAD encryption functions support the use of an `additional_data` argument,
also known as associated data (AD) or additional authenticated data.
A ciphertext can only be decrypted if the same additional data used to encrypt
is also provided to decrypt. The additional data can therefore be used
to bind the ciphertext to a context.

For example, `additional_data` could be the output of
`CAST(customer_id AS STRING)` when encrypting data for a particular customer.
This ensures that when the data is decrypted, it was previously encrypted using
the expected `customer_id`. The same `additional_data` value is required for
decryption. For more information, see
[RFC 5116](https://tools.ietf.org/html/rfc5116).

### Decryption

The output of [`AEAD.ENCRYPT`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeadencrypt) is
ciphertext `BYTES`. The
[`AEAD.DECRYPT_STRING`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeaddecrypt_string) or
[`AEAD.DECRYPT_BYTES`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#aeaddecrypt_bytes) functions can decrypt this
ciphertext. These functions must use a [keyset](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts#keysets) that
contains the key that was used for encryption. That key must be in an
`'ENABLED'` state. They must also use the same `additional_data` as was used in
encryption.

When the keyset is used for decryption, the appropriate key is chosen for
decryption based on the key ID encoded in the ciphertext.

The output of `AEAD.DECRYPT_STRING` is a plaintext
STRING, whereas the output of `AEAD.DECRYPT_BYTES` is
plaintext `BYTES`. `AEAD.DECRYPT_STRING` can decrypt
ciphertext that encodes a STRING value;
`AEAD.DECRYPT_BYTES` can decrypt ciphertext that encodes a
`BYTES` value. Using one of these functions to
decrypt a ciphertext that encodes the wrong data type, such as using
`AEAD.DECRYPT_STRING` to decrypt ciphertext that encodes a
`BYTES` value, causes undefined behavior and may
result in an error.

### Key rotation

The primary purpose of rotating encryption keys is to reduce the amount of
data encrypted with any particular key, so that a potential compromised key
would allow an attacker access to less data.

Keyset rotation involves:

1. Creating a new primary cryptographic key within every keyset.
2. Decrypting and re-encrypting all encrypted data.

The [`KEYS.ROTATE_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrotate_keyset) or
[`KEYS.ROTATE_WRAPPED_KEYSET`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keysrotate_wrapped_keyset)
function performs the first step, by adding a new primary cryptographic key to a
keyset and changing the old primary cryptographic key a secondary cryptographic
key.

### Cloud KMS keys

GoogleSQL supports [AEAD encryption functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions)
with [Cloud KMS keys](https://cloud.google.com/kms/docs/resource-hierarchy) to further secure your data. This
additional layer of protection encrypts your data encryption key (DEK) with a
key encryption key (KEK). The KEK is a symmetric encryption keyset that is
stored securely in the Cloud Key Management Service and managed using
[Cloud KMS permissions and roles](https://cloud.google.com/kms/docs/reference/permissions-and-roles#predefined).

At query execution time, use the [`KEYS.KEYSET_CHAIN`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions#keyskeyset_chain)
function to provide the KMS resource path of the KEK and the ciphertext from the
wrapped DEK. BigQuery calls Cloud KMS to unwrap the DEK, and then uses
that key to decrypt the data in your query. The unwrapped version of the DEK
is only stored in memory for the duration of the query, and then destroyed.

For more information, see
[SQL column-level encryption with Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/column-key-encrypt).