# Encryption at rest

By default, BigQuery encrypts customer content at
rest. BigQuery handles encryption for you without any
additional actions on your part. This option is called *Google default encryption* .
Google default encryption
uses the same hardened key management systems that we use for our own
encrypted data. These systems include strict key access controls and auditing.
Each BigQuery object's data and metadata is encrypted using the
[Advanced
Encryption Standard (AES)](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard).

If you want to control your encryption keys, then you can use customer-managed encryption keys
(CMEKs) in [Cloud KMS](https://docs.cloud.google.com/kms/docs) with CMEK-integrated services including
BigQuery. Using Cloud KMS keys gives you control over their protection
level, location, rotation schedule, usage and access permissions, and cryptographic boundaries.

Using Cloud KMS also lets
you [track key usage](https://docs.cloud.google.com/kms/docs/view-key-usage), view audit logs, and
control key lifecycles.


Instead of Google owning and managing the symmetric
[key encryption keys (KEKs)](https://docs.cloud.google.com/kms/docs/envelope-encryption#key_encryption_keys) that protect your data, you control and
manage these keys in Cloud KMS.

After you set up your resources with CMEKs, the experience of accessing your
BigQuery resources is similar to using Google default encryption.
For more information
about your encryption options, see [Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).

## CMEK with Cloud KMS Autokey

You can either create CMEKs manually to protect your BigQuery
resources or use Cloud KMS Autokey. With Autokey, key rings and keys are generated on demand to
support resource creation in BigQuery.
Service agents that use the keys for encrypt and decrypt operations are created if they don't
already exist and are granted the required Identity and Access Management (IAM) roles. For more
information, see [Autokey overview](https://docs.cloud.google.com/kms/docs/autokey-overview).


To learn how to use
manually-created CMEKs to protect your BigQuery resources, see
[Customer-managed Cloud KMS keys](https://docs.cloud.google.com/bigquery/docs/customer-managed-encryption).

To learn how to use CMEKs created by
Cloud KMS Autokey to protect your BigQuery resources,
see [Using Autokey with BigQuery
resources](https://docs.cloud.google.com/kms/docs/create-resource-with-autokey#bigquery-autokey).

<br />

## Encryption of individual values in a table

If you want to encrypt individual values within a BigQuery table,
use the Authenticated Encryption with Associated Data (AEAD) [encryption
functions](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/aead_encryption_functions). If you want to keep data for all of your own customers in a
common table, use AEAD functions to encrypt each customers' data using a
different key. The AEAD encryption functions are based on AES. For more
information, see [AEAD Encryption Concepts in GoogleSQL](https://docs.cloud.google.com/bigquery/docs/aead-encryption-concepts).

## Client-side encryption

Client-side encryption is separate from BigQuery encryption at
rest. If you choose to use client-side encryption, you are responsible for the
client-side keys and cryptographic operations. You would encrypt data before
writing it to BigQuery. In this case, your data is encrypted
twice, first with your keys and then with Google's keys. Similarly, data read
from BigQuery is decrypted twice, first with Google's keys and
then with your keys.

> [!IMPORTANT]
> **Important:** BigQuery does not know if your data has already been encrypted client-side, nor does BigQuery have any knowledge of your client-side encryption keys. If you use client-side encryption, you must securely manage your encryption keys and all aspects of client-side encryption and decryption.

## Data in transit

To protect your data as it travels over the Internet during read and write
operations, Google Cloud uses Transport Layer Security (TLS). For more
information, see [Encryption in transit in Google Cloud](https://docs.cloud.google.com/security/encryption-in-transit).

Within Google data centers, your data is encrypted when it is transferred
between machines.

## What's next

For more information about encryption at rest for BigQuery and
other Google Cloud products, see
[Encryption at rest in Google Cloud](https://docs.cloud.google.com/security/encryption/default-encryption).