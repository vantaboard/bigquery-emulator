# Perform semantic analysis with managed AI functions

This tutorial shows you how to use BigQuery ML managed AI functions to
perform semantic analysis on customer feedback.

## Objectives

In this tutorial, you:

- Create a dataset and load sentiment data into a table
- Use the following AI functions to perform semantic analysis:
  - [`AI.IF`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-if): to filter your data with natural language conditions
  - [`AI.SCORE`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-score): to rate input by sentiment
  - [`AI.CLASSIFY`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-ai-classify): to classify input into user-defined categories

## Costs

This tutorial uses billable components of Google Cloud,
including the following:

- BigQuery
- BigQuery ML

For more information on BigQuery costs, see the
[BigQuery pricing](https://cloud.google.com/bigquery/pricing) page.

For more information on BigQuery ML costs, see
[BigQuery ML pricing](https://cloud.google.com/bigquery/pricing#bqml).

## Before you begin

1.


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

   For new projects, the BigQuery API is
   automatically enabled.
2. Optional: [Enable
   billing](https://docs.cloud.google.com/billing/docs/how-to/modify-project) for the project. If you don't want to enable billing or provide a credit card, the steps in this document still work. BigQuery provides you a sandbox to perform the steps. For more information, see [Enable the BigQuery sandbox](https://docs.cloud.google.com/bigquery/docs/sandbox#setup).

   > [!NOTE]
   > **Note:** If your project has a billing account and you want to use the BigQuery sandbox, then [disable billing for your project](https://docs.cloud.google.com/billing/docs/how-to/modify-project#disable_billing_for_a_project).

### Required roles


To get the permissions that
you need to use AI functions,

ask your administrator to grant you the
following IAM roles on the project:

- Run query jobs and load jobs: [BigQuery Job User](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.jobUser) (`roles/bigquery.jobUser`)
- Create a dataset, create a table, load data into a table, and query a table: [BigQuery Data Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataEditor) (`roles/bigquery.dataEditor`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Create sample data

To create a dataset called `my_dataset` for this tutorial, run the following
query.

    CREATE SCHEMA my_dataset OPTIONS (location = 'LOCATION');

Next, create a table called `customer_feedback` that contains sample customer
reviews for a device:

    CREATE TABLE my_dataset.customer_feedback AS (
      SELECT
        *
      FROM
        UNNEST( [STRUCT<review_id INT64, review_text STRING> 
          (1, "The battery life is incredible, and the screen is gorgeous! Best phone I've ever had. Totally worth the price."),
          (2, "Customer support was a nightmare. It took three weeks for my order to arrive, and when it did, the box was damaged. Very frustrating!"),
          (3, "The product does exactly what it says on the box. No complaints, but not exciting either."),
          (4, "I'm so happy with this purchase! It arrived early and exceeded all my expectations. The quality is top-notch, although the setup was a bit tricky."),
          (5, "The price is a bit too high for what you get. The material feels cheap and I'm worried it won't last. Service was okay."),
          (6, "Absolutely furious! The item arrived broken, and getting a refund is proving impossible. I will never buy from them again."),
          (7, "This new feature for account access is confusing. I can't find where to update my profile. Please fix this bug!"),
          (8, "The shipping was delayed, but the support team was very helpful and kept me informed. The product itself is great, especially for the price.") 
          ])
    );

## Categorize overall sentiment

It can be helpful to extract the overall sentiment expressed in text
to support use cases such as the following:

- Gauge customer satisfaction from reviews.
- Monitor brand perception on social media.
- Prioritize support tickets based on how upset users are.

The following query shows how to use the `AI.CLASSIFY` function to classify
reviews from the `customer_feedback` table as *positive* , *negative* , or
*neutral*:

    SELECT
      review_id,
      review_text,
      AI.CLASSIFY(
        review_text,
        categories => ['positive', 'negative', 'neutral']) AS sentiment
    FROM
      my_dataset.customer_feedback;

The result looks similar to the following:

```
+---+---+---+
| review_id | review_text                              | sentiment |
+---+---+---+
| 7         | This new feature for account access is   | negative  |
|           | confusing. I can't find where to update  |           |
|           | my profile. Please fix this bug!         |           |
+---+---+---+
| 4         | "I'm so happy with this purchase! It     | positive  |
|           | arrived early and exceeded all my        |           |
|           | expectations. The quality is top-notch,  |           |
|           | although the setup was a bit tricky."    |           |
+---+---+---+
| 2         | "Customer support was a nightmare. It    | negative  |
|           | took three weeks for my order to         |           |
|           | arrive, and when it did, the box was     |           |
|           | damaged. Very frustrating!"              |           |
+---+---+---+
| 1         | "The battery life is incredible, and     | positive  |
|           | the screen is gorgeous! Best phone I've  |           |
|           | ever had. Totally worth the price."      |           |
+---+---+---+
| 8         | "The shipping was delayed, but the       | positive  |
|           | support team was very helpful and kept   |           |
|           | me informed. The product itself is       |           |
|           | great, especially for the price."        |           |
+---+---+---+
| 5         | The price is a bit too high for what     | negative  |
|           | you get. The material feels cheap and    |           |
|           | I'm worried it won't last. Service was   |           |
|           | okay.                                    |           |
+---+---+---+
| 3         | "The product does exactly what it says   | neutral   |
|           | on the box. No complaints, but not       |           |
|           | exciting either."                        |           |
+---+---+---+
| 6         | "Absolutely furious! The item arrived    | negative  |
|           | broken, and getting a refund is proving  |           |
|           | impossible. I will never buy from them   |           |
|           | again."                                  |           |
+---+---+---+
```

## Analyze aspect-based sentiment

If an overall sentiment such as *positive* or *negative* isn't sufficient for
your use case, you can analyze a specific aspect of the meaning of text. For
example, you might want to understand a user's attitude towards the quality
of the product, without regard for their thoughts on its price. You can even
ask for a custom value to indicate that a particular aspect doesn't apply.

The following example shows how to use the `AI.SCORE` function to rate user
sentiment from 1 to 10 based on how favorable each review in the
`customer_feedback` table is toward price, customer service, and quality. The
function returns the custom value -1 in cases where an aspect isn't mentioned in
the review so that you can filter these out later.

    SELECT
      review_id,
      review_text,
      AI.SCORE(
        ("Score 0.0 to 10 on positive sentiment about PRICE for review: ", review_text,
        "If price is not mentioned, return -1.0")) AS price_score,
      AI.SCORE(
        ("Score 0.0 to 10 on positive sentiment about CUSTOMER SERVICE for review: ", review_text,
        "If customer service is not mentioned, return -1.0")) AS service_score,
      AI.SCORE(
        ("Score 0.0 to 10 on positive sentiment about QUALITY for review: ", review_text,
        "If quality is not mentioned, return -1.0")) AS quality_score
    FROM
      my_dataset.customer_feedback
    LIMIT 3;

The result looks similar to the following:

```
+---+---+---+---+---+
| review_id | review_text                              |  price_score | service_score | quality_score |
+---+---+---+---+---+
| 4         | "I'm so happy with this purchase! It     | -1.0         | -1.0          | 9.5           |
|           | arrived early and exceeded all my        |              |               |               |
|           | expectations. The quality is top-notch,  |              |               |               |
|           | although the setup was a bit tricky."    |              |               |               |
+---+---+---+---+---+
| 8         | "The shipping was delayed, but the       |  9.0         |  8.5          | 9.0           |
|           | support team was very helpful and kept   |              |               |               |
|           | me informed. The product itself is       |              |               |               |
|           | great, especially for the price."        |              |               |               |
+---+---+---+---+---+
| 6         | "Absolutely furious! The item arrived    | -1.0         |  1.0          | 0.0           |
|           | broken, and getting a refund is proving  |              |               |               |
|           | impossible. I will never buy from them   |              |               |               |
|           | again."                                  |              |               |               |
+---+---+---+---+---+
```

## Detect emotions

In addition to positive or negative sentiment, you can classify text based on
specific emotions that you select. This is useful when you want to gain a
better understanding of user responses, or to flag highly emotional feedback
for review.

    SELECT
      review_id,
      review_text,
      AI.CLASSIFY(
        review_text,
        categories => ['joy', 'anger', 'sadness', 'surprise', 'fear', 'disgust', 'neutral', 'other']
      ) AS emotion
    FROM
      my_dataset.customer_feedback;

The result looks similar to the following:

```
+---+---+---+
| review_id | review_text                              | emotion |
+---+---+---+
| 2         | "Customer support was a nightmare. It    | anger   |
|           | took three weeks for my order to         |         |
|           | arrive, and when it did, the box was     |         |
|           | damaged. Very frustrating!"              |         |
+---+---+---+
| 7         | This new feature for account access is   | anger   |
|           | confusing. I can't find where to update  |         |
|           | my profile. Please fix this bug!         |         |
+---+---+---+
| 4         | "I'm so happy with this purchase! It     | joy     |
|           | arrived early and exceeded all my        |         |
|           | expectations. The quality is top-notch,  |         |
|           | although the setup was a bit tricky."    |         |
+---+---+---+
| 1         | "The battery life is incredible, and     | joy     |
|           | the screen is gorgeous! Best phone I've  |         |
|           | ever had. Totally worth the price."      |         |
+---+---+---+
| 8         | "The shipping was delayed, but the       | joy     |
|           | support team was very helpful and kept   |         |
|           | me informed. The product itself is       |         |
|           | great, especially for the price."        |         |
+---+---+---+
| 5         | The price is a bit too high for what     | sadness |
|           | you get. The material feels cheap and    |         |
|           | I'm worried it won't last. Service was   |         |
|           | okay.                                    |         |
+---+---+---+
| 3         | "The product does exactly what it says   | neutral |
|           | on the box. No complaints, but not       |         |
|           | exciting either."                        |         |
+---+---+---+
| 6         | "Absolutely furious! The item arrived    | anger   |
|           | broken, and getting a refund is proving  |         |
|           | impossible. I will never buy from them   |         |
|           | again."                                  |         |
+---+---+---+
```

## Categorize reviews by topic

You can use the `AI.CLASSIFY` function to group reviews into predefined topics.
For example, you can do the following:

- Discover common themes in customer feedback.
- Organize documents by subject matter.
- Route support tickets by topic.

The following example shows how to classify customer feedback into various
types such as *billing issue* or *account access* and then count how many
reviews belong to each category:

    SELECT
      AI.CLASSIFY(
        review_text,
        categories => ['Billing Issue', 'Account Access',
                       'Product Bug', 'Feature Request',
                       'Shipping Delay', 'Other']) AS topic,
        COUNT(*) AS number_of_reviews,
    FROM
      my_dataset.customer_feedback
    GROUP BY topic
    ORDER BY number_of_reviews DESC;

The result looks similar to the following:

```
+---+---+
| topic          | number_of_reviews |
+---+---+
| Other          | 5                 |
| Shipping Delay | 2                 |
| Product Bug    | 1                 |
+---+---+
```

## Identify semantically similar reviews

You can use the `AI.SCORE` function to assess how semantically similar two
pieces of text are by asking it to rate similarity of meaning. This can help you
with tasks such as the following:

- Find duplicate or near-duplicate entries.
- Group similar pieces of feedback.
- Power semantic search applications.

The following query finds reviews that discuss difficulty setting up the
product:

    SELECT
      review_id,
      review_text,
      AI.SCORE(
        (
          """How similar is the review to the concept of 'difficulty in setting up the product'?
             A higher score indicates more similarity. Review: """,
          review_text)) AS setup_difficulty
    FROM my_dataset.customer_feedback
    ORDER BY setup_difficulty DESC
    LIMIT 2;

The result looks similar to the following:

```
+---+---+---+
| review_id | review_text                              | setup_difficulty |
+---+---+---+
| 4         | "I'm so happy with this purchase! It     | 3                |
|           | arrived early and exceeded all my        |                  |
|           | expectations. The quality is top-notch,  |                  |
|           | although the setup was a bit tricky."    |                  |
+---+---+---+
| 7         | This new feature for account access is   | 1                |
|           | confusing. I can't find where to update  |                  |
|           | my profile. Please fix this bug!         |                  |
+---+---+---+
```

You can also use the `AI.IF` function to find reviews that relate to text:

    SELECT
      review_id,
      review_text
    FROM my_dataset.customer_feedback
    WHERE
      AI.IF(
        (
          "Does this review discuss difficulty setting up the product? Review: ",
          review_text));

## Combine functions

It can be helpful to combine these functions in a single query. For example,
the following query first filters reviews for negative sentiment, and then
classifies them by the type of frustration:

    SELECT
      review_id,
      review_text,
      AI.CLASSIFY(
        review_text,
        categories => [
          'Poor Quality', 'Bad Customer Service', 'High Price', 'Other Negative']) AS negative_topic
    FROM my_dataset.customer_feedback
    WHERE
      AI.IF(
        ("Does this review express a negative sentiment? Review: ", review_text));

## Create reusable prompt UDFs

To keep your queries readable, you can reuse your prompt logic by creating
[user-defined functions](https://docs.cloud.google.com/bigquery/docs/user-defined-functions). The following
query creates a function to detect negative sentiment by calling `AI.IF` with
a custom prompt. Then, it calls that function to filter by negative review.

    CREATE OR REPLACE FUNCTION my_dataset.is_negative_sentiment(review_text STRING)
    RETURNS BOOL
    AS (
        AI.IF(
          ("Does this review express a negative sentiment? Review: ", review_text))
    );

    SELECT
      review_id,
      review_text
    FROM my_dataset.customer_feedback
    WHERE my_dataset.is_negative_sentiment(review_text);

## Clean up

To avoid incurring charges, you can either delete the project that contains the
resources that you created, or keep the project and delete the individual
resources.

### Delete your project

To delete the project:

> [!CAUTION]
> **Caution** : Deleting a project has the following effects:
>
> - **Everything in the project is deleted.** If you used an existing project for the tasks in this document, when you delete it, you also delete any other work you've done in the project.
> - **Custom project IDs are lost.** When you created this project, you might have created a custom project ID that you want to use in the future. To preserve the URLs that use the project ID, such as an `appspot.com` URL, delete selected resources inside the project instead of deleting the whole project.
>
>
> If you plan to explore multiple architectures, tutorials, or quickstarts, reusing projects
> can help you avoid exceeding project quota limits.

1. In the Google Cloud console, go to the **Manage resources** page.

   [Go to Manage resources](https://console.cloud.google.com/iam-admin/projects)
2. In the project list, select the project that you want to delete, and then click **Delete**.
3. In the dialog, type the project ID, and then click **Shut down** to delete the project.

<br />

### Delete your dataset

To delete the dataset and all resources that it contains, including all tables
and functions, run the following query:

    DROP SCHEMA my_dataset CASCADE;