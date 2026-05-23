This tutorial teaches you how to create a
[matrix factorization model](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization)
and train it on the customer movie ratings in the
[`movielens1m`](https://grouplens.org/datasets/movielens/1m/) dataset. You then
use the matrix factorization model to generate movie recommendations for users.

Using customer-provided ratings to train the model is called
training with *explicit feedback* . Matrix factorization models are trained
using the
[Alternating Least Squares algorithm](https://en.wikipedia.org/wiki/Matrix_completion#Alternating_least_squares_minimization) when you use
explicit feedback as training data.

> [!IMPORTANT]
> **Important:** You must have a reservation in order to use a matrix factorization model. For more information, see [Pricing](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-create-matrix-factorization#pricing).

## Objectives

This tutorial guides you through completing the following tasks:

- Creating a matrix factorization model by using the `CREATE MODEL` statement.
- Evaluating the model by using the [`ML.EVALUATE` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate).
- Generating movie recommendations for users by using the model with the [`ML.RECOMMEND` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-recommend).

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

1. BigQuery is automatically enabled in new projects. To activate BigQuery in a pre-existing project, go to


   Enable the BigQuery API.


   **Roles required to enable APIs**


   To enable APIs, you need the Service Usage Admin IAM
   role (`roles/serviceusage.serviceUsageAdmin`), which
   contains the `serviceusage.services.enable` permission. [Learn how to grant
   roles](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).

   [Enable the API](https://console.cloud.google.com/flows/enableapi?apiid=bigquery)

## Required Permissions

- To create the dataset, you need the `bigquery.datasets.create`
  IAM permission.

- To create the model, you need the following permissions:

  - `bigquery.jobs.create`
  - `bigquery.models.create`
  - `bigquery.models.getData`
  - `bigquery.models.updateData`
- To run inference, you need the following permissions:

  - `bigquery.models.getData`
  - `bigquery.jobs.create`

For more information about IAM roles and permissions in
BigQuery, see
[Introduction to IAM](https://docs.cloud.google.com/bigquery/docs/access-control).

## Create a dataset

Create a BigQuery dataset to store your ML model.

<br />

### Console

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the **Explorer** pane, click your project name.

3. Click **View actions \> Create dataset**

4. On the **Create dataset** page, do the following:

   - For **Dataset ID** , enter `bqml_tutorial`.

   - For **Location type** , select **Multi-region** , and then select
     **US**.

   - Leave the remaining default settings as they are, and click
     **Create dataset**.

### bq

To create a new dataset, use the
[`bq mk --dataset` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#mk-dataset).

1. Create a dataset named `bqml_tutorial` with the data location set to `US`.

   ```
   bq mk --dataset \
     --location=US \
     --description "BigQuery ML tutorial dataset." \
     bqml_tutorial
   ```
2. Confirm that the dataset was created:

   ```bash
   bq ls
   ```

### API

Call the [`datasets.insert`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
method with a defined [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

<br />

```json
{
  "datasetReference": {
     "datasetId": "bqml_tutorial"
  }
}
```

<br />

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).


    import google.cloud.bigquery

    bqclient = google.cloud.https://docs.cloud.google.com/python/docs/reference/bigquery/latest.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html()
    bqclient.https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client.html#google_cloud_bigquery_client_Client_create_dataset("bqml_tutorial", exists_ok=True)

## Upload the Movielens data

Upload the `movielens1m` data into BigQuery.

### CLI

Follow these steps to upload the `movielens1m` data using the
[bq command-line tool](https://docs.cloud.google.com/bigquery/docs/bq-command-line-tool):

1. Open Cloud Shell:

   [Activate Cloud Shell](https://console.cloud.google.com/bigquery?cloudshell=true)
2. Upload the ratings data into the `ratings` table. On the command line, paste
   in the following query and hit `Enter`:

       curl -O 'http://files.grouplens.org/datasets/movielens/ml-1m.zip'
       unzip ml-1m.zip
       sed 's/::/,/g' ml-1m/ratings.dat > ratings.csv
       bq load --source_format=CSV bqml_tutorial.ratings ratings.csv \
         user_id:INT64,item_id:INT64,rating:FLOAT64,timestamp:TIMESTAMP

3. Upload the movie data into the `movies` table. On the command line,
   paste in the following query and hit `Enter`:

       sed 's/::/@/g' ml-1m/movies.dat > movie_titles.csv
       bq load --source_format=CSV --field_delimiter=@ \
       bqml_tutorial.movies movie_titles.csv \
       movie_id:INT64,movie_title:STRING,genre:STRING

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

First, create a `Client` object with
`bqclient = google.cloud.bigquery.Client()`, then load the `movielens1m` data
into the dataset you created in the previous step.

    import io
    import zipfile

    import google.api_core.exceptions
    import requests

    try:
        # Check if you've already created the Movielens tables to avoid downloading
        # and uploading the dataset unnecessarily.
        bqclient.get_table("bqml_tutorial.ratings")
        bqclient.get_table("bqml_tutorial.movies")
    except google.api_core.exceptions.NotFound:
        # Download the https://grouplens.org/datasets/movielens/1m/ dataset.
        ml1m = requests.get("http://files.grouplens.org/datasets/movielens/ml-1m.zip")
        ml1m_file = io.BytesIO(ml1m.content)
        ml1m_zip = zipfile.ZipFile(ml1m_file)

        # Upload the ratings data into the ratings table.
        with ml1m_zip.open("ml-1m/ratings.dat") as ratings_file:
            ratings_content = ratings_file.read()

        ratings_csv = io.BytesIO(ratings_content.replace(b"::", b","))
        ratings_config = google.cloud.bigquery.LoadJobConfig()
        ratings_config.source_format = "CSV"
        ratings_config.write_disposition = "WRITE_TRUNCATE"
        ratings_config.schema = [
            google.cloud.bigquery.SchemaField("user_id", "INT64"),
            google.cloud.bigquery.SchemaField("item_id", "INT64"),
            google.cloud.bigquery.SchemaField("rating", "FLOAT64"),
            google.cloud.bigquery.SchemaField("timestamp", "TIMESTAMP"),
        ]
        bqclient.load_table_from_file(
            ratings_csv, "bqml_tutorial.ratings", job_config=ratings_config
        ).result()

        # Upload the movie data into the movies table.
        with ml1m_zip.open("ml-1m/movies.dat") as movies_file:
            movies_content = movies_file.read()

        movies_csv = io.BytesIO(movies_content.replace(b"::", b"@"))
        movies_config = google.cloud.bigquery.LoadJobConfig()
        movies_config.source_format = "CSV"
        movies_config.field_delimiter = "@"
        movies_config.write_disposition = "WRITE_TRUNCATE"
        movies_config.schema = [
            google.cloud.bigquery.SchemaField("movie_id", "INT64"),
            google.cloud.bigquery.SchemaField("movie_title", "STRING"),
            google.cloud.bigquery.SchemaField("genre", "STRING"),
        ]
        bqclient.load_table_from_file(
            movies_csv, "bqml_tutorial.movies", job_config=movies_config
        ).result()

## Create the model

Create a matrix factorization model and train it on the data in the `ratings`
table. The model is trained to predict a rating for every user-item pair,
based on the customer-provided movie ratings.

### SQL

The following `CREATE MODEL` statement uses these columns to generate
recommendations:

- `user_id`---The user ID.
- `item_id`---The movie ID.
- `rating`---The explicit rating from 1 to 5 that the user gave the item.

Follow these steps to create the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE MODEL `bqml_tutorial.mf_explicit`
   OPTIONS (
     MODEL_TYPE = 'matrix_factorization',
     FEEDBACK_TYPE = 'explicit',
     USER_COL = 'user_id',
     ITEM_COL = 'item_id',
     L2_REG = 9.83,
     NUM_FACTORS = 34)
   AS
   SELECT
   user_id,
   item_id,
   rating
   FROM `bqml_tutorial.ratings`;
   ```

   The query takes about 10 minutes to complete, after which the
   `mf_explicit` model appears in the **Explorer** pane. Because
   the query uses a `CREATE MODEL` statement to create a model, you don't see
   query results.

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

    from bigframes.ml import decomposition
    import bigframes.pandas as bpd

    # Load data from BigQuery
    bq_df = bpd.read_gbq(
        "bqml_tutorial.ratings", columns=("user_id", "item_id", "rating")
    )

    # Create the Matrix Factorization model
    model = decomposition.MatrixFactorization(
        num_factors=34,
        feedback_type="explicit",
        user_col="user_id",
        item_col="item_id",
        rating_col="rating",
        l2_reg=9.83,
    )
    model.fit(bq_df)
    model.to_gbq(
        your_model_id, replace=True  # For example: "bqml_tutorial.mf_explicit"
    )

The code takes about 10 minutes to complete, after which the
`mf_explicit` model appears in the **Explorer** pane.

## Get training statistics

Optionally, you can view the model's training statistics in the
Google Cloud console.

A machine learning algorithm builds a model by creating many iterations of
the model using different parameters, and then selecting the version of the
model that minimizes [loss](https://en.wikipedia.org/wiki/Loss_function).
This process is called empirical risk minimization. The model's training
statistics let you see the loss associated with each iteration of the model.

Follow these steps to view the model's training statistics:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets** , and then
   click the `bqml_tutorial` dataset.

4. Click the **Models** tab.

5. Click the `mf_explicit` model and then click the **Training** tab

6. In the **View as** section, click **Table**. The results should look
   similar to the following:

   ```
   +---+---+---+
   | Iteration | Training Data Loss | Duration (seconds) |
   +---+---+---+
   |  11       | 0.3943             | 42.59              |
   +---+---+---+
   |  10       | 0.3979             | 27.37              |
   +---+---+---+
   |   9       | 0.4038             | 40.79              |
   +---+---+---+
   |  ...      | ...                | ...                |
   +---+---+---+
   ```

   The **Training Data Loss** column represents the loss metric calculated
   after the model is trained. Because this is a matrix factorization model,
   this column shows the
   [mean squared error](https://developers.google.com/machine-learning/glossary/#MSE).

You can also use the
[`ML.TRAINING_INFO` function](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-train)
to see model training statistics.

## Evaluate the model

Evaluate the performance of the model by comparing the predicted movie ratings
returned by the model against the actual user movie ratings from the training
data.

### SQL

Use the `ML.EVALUATE` function to evaluate the model:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
   *
   FROM
   ML.EVALUATE(
     MODEL `bqml_tutorial.mf_explicit`,
     (
       SELECT
         user_id,
         item_id,
         rating
       FROM
         `bqml_tutorial.ratings`
     ));
   ```

   The results should look similar to the following:

   ```
   +---+---+---+---+---+---+
   | mean_absolute_error | mean_squared_error  | mean_squared_log_error | median_absolute_error |      r2_score      | explained_variance |
   +---+---+---+---+---+---+
   | 0.48494444327829156 | 0.39433706592870565 |   0.025437895793637522 |   0.39017059802629905 | 0.6840033369412044 | 0.6840033369412264 |
   +---+---+---+---+---+---+
   ```

   An important metric in the evaluation results is the
   [R^2^
   score](https://en.wikipedia.org/wiki/Coefficient_of_determination).
   The R^2^ score is a statistical measure that determines if the
   linear regression predictions approximate the actual data. A value of `0`
   indicates that the model explains none of the variability of the
   response data around the mean. A value of `1` indicates that the model
   explains all the variability of the response data around the mean.

   For more information about the `ML.EVALUATE` function output, see
   [Output](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/bigqueryml-syntax-evaluate#output).

You can also call `ML.EVALUATE` without providing the input data. It will
use the evaluation metrics calculated during training.

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

Call
[`model.score()`](https://dataframes.bigquery.dev/reference/api/bigframes.ml.decomposition.MatrixFactorization#bigframes.ml.decomposition.MatrixFactorization.score)
to evaluate the model.

    # Evaluate the model using the score() function
    model.score(bq_df)
    # Output:
    # mean_absolute_error	mean_squared_error	mean_squared_log_error	median_absolute_error	r2_score	explained_variance
    # 0.485403	                0.395052	        0.025515	            0.390573	        0.68343	        0.68343

## Get the predicted ratings for a subset of user-item pairs

Get the predicted rating for each movie for five users.

### SQL

Use the `ML.RECOMMEND` function to get predicted ratings:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, paste in the following query and click **Run**:

   ```googlesql
   SELECT
   *
   FROM
   ML.RECOMMEND(
     MODEL `bqml_tutorial.mf_explicit`,
     (
       SELECT
         user_id
       FROM
         `bqml_tutorial.ratings`
       LIMIT 5
     ));
   ```

   The results should look similar to the following:

   ```
   +---+---+---+
   | predicted_rating   | user_id | item_id |
   +---+---+---+
   | 4.2125303962491873 | 4       | 3169    |
   +---+---+---+
   | 4.8068920531981263 | 4       | 3739    |
   +---+---+---+
   | 3.8742203494732403 | 4       | 3574    |
   +---+---+---+
   | ...                | ...     | ...     |
   +---+---+---+
   ```

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

Call
[`model.predict()`](https://dataframes.bigquery.dev/reference/api/bigframes.ml.decomposition.MatrixFactorization#bigframes.ml.decomposition.MatrixFactorization.predict)
to get predicted ratings.

    # Use predict() to get the predicted rating for each movie for 5 users
    subset = bq_df[["user_id"]].head(5)
    predicted = model.predict(subset)
    print(predicted)
    # Output:
    #   predicted_rating	user_id	 item_id	rating
    # 0	    4.206146	     4354	  968	     4.0
    # 1	    4.853099	     3622	  3521	     5.0
    # 2	    2.679067	     5543	  920	     2.0
    # 3	    4.323458	     445	  3175	     5.0
    # 4	    3.476911	     5535	  235	     4.0

## Generate recommendations

Use the predicted ratings to generate the top five recommended movies for
each user.

### SQL

Follow these steps to generate recommendations:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. Write the predicted ratings to a table. In the query editor, paste in the
   following query and click **Run**:

   ```googlesql
   CREATE OR REPLACE TABLE `bqml_tutorial.recommend`
   AS
   SELECT
   *
   FROM
   ML.RECOMMEND(MODEL `bqml_tutorial.mf_explicit`);
   ```
3. Join the predicted ratings with the movie information, and select the top
   five results per user. In the query editor, paste in the
   following query and click **Run**:

```googlesql
  SELECT
    user_id,
    ARRAY_AGG(STRUCT(movie_title, genre, predicted_rating) ORDER BY predicted_rating DESC LIMIT 5)
  FROM
    (
      SELECT
        user_id,
        item_id,
        predicted_rating,
        movie_title,
        genre
      FROM
        `bqml_tutorial.recommend`
      JOIN
        `bqml_tutorial.movies`
        ON
          item_id = movie_id
    )
  GROUP BY
    user_id;
```

The results should look similar to the following:

<br />

```
  +---+---+---+---+
  | user_id | f0_movie_title                      | f0_genre               | predicted_rating   |
  +---+---+---+---+
  | 4597    | Song of Freedom (1936)              | Drama                  | 6.8495752907364009 |
  |         | I Went Down (1997)                  | Action/Comedy/Crime    | 6.7203235758772877 |
  |         | Men With Guns (1997)                | Action/Drama           | 6.399407352232001  |
  |         | Kid, The (1921)                     | Action                 | 6.1952890198126731 |
  |         | Hype! (1996)                        | Documentary            | 6.1895766097451475 |
  +---+---+---+---+
  | 5349    | Fandango (1985)                     | Comedy                 | 9.944574012151549  |
  |         | Breakfast of Champions (1999)       | Comedy                 | 9.55661860430112   |
  |         | Funny Bones (1995)                  | Comedy                 | 9.52778917835076   |
  |         | Paradise Road (1997)                | Drama/War              | 9.1643621767929133 |
  |         | Surviving Picasso (1996)            | Drama                  | 8.807353289233772  |
  +---+---+---+---+
  | ...     | ...                                 | ...                    | ...                |
  +---+---+---+---+
  
```

<br />

### BigQuery DataFrames

<br />

Before trying this sample, follow the BigQuery DataFrames
setup instructions in the [BigQuery quickstart
using BigQuery DataFrames](https://docs.cloud.google.com/bigquery/docs/dataframes-quickstart).
For more information, see the
[BigQuery DataFrames reference documentation](https://docs.cloud.google.com/python/docs/reference/bigframes/latest).

To authenticate to BigQuery, set up Application Default Credentials.
For more information, see [Set
up ADC for a local development environment](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment).

Call
[`model.predict()`](https://dataframes.bigquery.dev/reference/api/bigframes.ml.decomposition.MatrixFactorization#bigframes.ml.decomposition.MatrixFactorization.predict)
to get predicted ratings.

    # import bigframes.bigquery as bbq

    # Load movies
    movies = bpd.read_gbq("bqml_tutorial.movies")

    # Merge the movies df with the previously created predicted df
    merged_df = bpd.merge(predicted, movies, left_on="item_id", right_on="movie_id")

    # Separate users and predicted data, setting the index to 'movie_id'
    users = merged_df[["user_id", "movie_id"]].set_index("movie_id")

    # Take the predicted data and sort it in descending order by 'predicted_rating', setting the index to 'movie_id'
    sort_data = (
        merged_df[["movie_title", "genre", "predicted_rating", "movie_id"]]
        .sort_values(by="predicted_rating", ascending=False)
        .set_index("movie_id")
    )

    # re-merge the separated dfs by index
    merged_user = sort_data.join(users, how="outer")

    # group the users and set the user_id as the index
    merged_user.groupby("user_id").head(5).set_index("user_id").sort_index()
    print(merged_user)
    # Output:
    # 	            movie_title	                genre	        predicted_rating
    # user_id
    #   1	    Saving Private Ryan (1998)	Action|Drama|War	    5.19326
    #   1	        Fargo (1996)	       Crime|Drama|Thriller	    4.996954
    #   1	    Driving Miss Daisy (1989)	    Drama	            4.983671
    #   1	        Ben-Hur (1959)	       Action|Adventure|Drama	4.877622
    #   1	     Schindler's List (1993)	   Drama|War	        4.802336
    #   2	    Saving Private Ryan (1998)	Action|Drama|War	    5.19326
    #   2	        Braveheart (1995)	    Action|Drama|War	    5.174145
    #   2	        Gladiator (2000)	      Action|Drama	        5.066372
    #   2	        On Golden Pond (1981)	     Drama	            5.01198
    #   2	    Driving Miss Daisy (1989)	     Drama	            4.983671

## Clean up


To avoid incurring charges to your Google Cloud account for the resources used in this
tutorial, either delete the project that contains the resources, or keep the project and
delete the individual resources.

- You can delete the project you created.
- Or you can keep the project and delete the dataset.

### Delete your dataset

Deleting your project removes all datasets and all tables in the project. If you
prefer to reuse the project, you can delete the dataset you created in this
tutorial:

1. If necessary, open the BigQuery page in the Google Cloud console.

   [Go to the BigQuery page](https://console.cloud.google.com/bigquery)
2. In the navigation, click the **bqml_tutorial** dataset you created.

3. Click **Delete dataset** on the right side of the window.
   This action deletes the dataset, the table, and all the data.

4. In the **Delete dataset** dialog, confirm the delete command by typing
   the name of your dataset (`bqml_tutorial`) and then click **Delete**.

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

## What's next

- Try [creating a matrix factorization model based on implicit feedback](https://docs.cloud.google.com/bigquery/docs/bigqueryml-mf-implicit-tutorial).
- For an overview of BigQuery ML, see [Introduction to BigQuery ML](https://docs.cloud.google.com/bigquery/docs/bqml-introduction).
- To learn more about machine learning, see the [Machine learning crash course](https://developers.google.com/machine-learning/crash-course/).