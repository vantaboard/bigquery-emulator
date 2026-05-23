# Control access to resources with IAM

This document describes how to view, grant, and revoke access controls for
BigQuery datasets and for the resources within datasets:
tables, views, and routines. Although models are also dataset-level resources,
you cannot grant access to individual models using IAM roles.

You can grant access to Google Cloud resources by using
*allow policies* , also known as *Identity and Access Management (IAM) policies* , which are
attached to resources. You can attach only one allow policy to each resource.
The allow policy controls access to the resource itself, as well as any
descendants of that resource that [inherit the allow policy](https://docs.cloud.google.com/iam/docs/allow-policies#inheritance).

For more information on allow policies, see [Policy structure](https://docs.cloud.google.com/iam/docs/allow-policies#structure)
in the IAM documentation.

This document assumes familiarity with the
[Identity and Access Management (IAM)](https://docs.cloud.google.com/iam/docs/overview) in Google Cloud.

## Limitations

- Routine access control lists (ACLs) aren't included in [replicated routines](https://docs.cloud.google.com/bigquery/docs/data-replication).
- Routines inside external or linked datasets don't support access controls.
- Tables inside external or linked datasets don't support access controls.
- Routine access controls can't be set with Terraform.
- Routine access controls can't be set with the Google Cloud SDK.
- Routine access controls can't be set using the [BigQuery data control language (DCL)](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language).
- Data Catalog doesn't support routine access controls. If a user has conditionally granted routine-level access, they won't see their routines in the BigQuery side panel. As a workaround, grant dataset-level access instead.
- The [`INFORMATION_SCHEMA.OBJECT_PRIVILEGES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges) doesn't show access controls for routines.

## Before you begin

Grant Identity and Access Management (IAM) roles that give users the necessary permissions
to perform each task in this document.

### Required roles


To get the permissions that
you need to modify IAM policies for resources,

ask your administrator to grant you the
[BigQuery Data Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/bigquery#bigquery.dataOwner) (`roles/bigquery.dataOwner`) IAM role on the project.


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


This predefined role contains

the permissions required to modify IAM policies for resources. To see the exact permissions that are
required, expand the **Required permissions** section:


#### Required permissions

The following permissions are required to modify IAM policies for resources:

- To get a dataset's access policy: `bigquery.datasets.get`
- To set a dataset's access policy: `bigquery.datasets.update`
- To get a dataset's access policy (Google Cloud console only): `bigquery.datasets.getIamPolicy`
- To set a dataset's access policy (console only): `bigquery.datasets.setIamPolicy`
- To get a table or view's policy: `bigquery.tables.getIamPolicy`
- To set a table or view's policy: `bigquery.tables.setIamPolicy`
- To get a routine's access policy: `bigquery.routines.getIamPolicy`
- To set a routine's access policy: `bigquery.routines.setIamPolicy`
- To create bq tool or [SQL BigQuery jobs](https://docs.cloud.google.com/bigquery/docs/managing-jobs) (optional): `bigquery.jobs.create`


You might also be able to get
these permissions
with [custom roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or
other [predefined roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

## Work with dataset access controls

You can provide access to a dataset by granting an [IAM principal](https://docs.cloud.google.com/iam/docs/principal-identifiers#allow)
a predefined or custom role that determines what the principal can do with the
dataset. This is also known as attaching an [allow policy](https://docs.cloud.google.com/iam/docs/allow-policies)
to a resource. After granting access, you can view the dataset's access
controls, and you can revoke access to the dataset.

### Grant access to a dataset

You can't grant access to a dataset when you create it using the BigQuery web UI or
the bq command-line tool. You must create the dataset first and then grant access to it.
The API lets you grant access during dataset creation by calling the
[`datasets.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
with a defined [dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

A project is the parent resource for a dataset, and a dataset is the parent
resource for tables and views, routines, and models. When you grant a role at
the project level, the role and its permissions are inherited by the dataset and
by the dataset's resources. Similarly, when you grant a role at the dataset
level, the role and its permissions are inherited by the resources within the
dataset.

You can provide access to a dataset by granting an IAM role
permission to access the dataset or by conditionally granting access using
an IAM condition. For more information on granting conditional
access, see [Control access with IAM Conditions](https://docs.cloud.google.com/bigquery/docs/conditions#add-conditions-to-datasets).

> [!NOTE]
> **Note:** Granting access to a dataset doesn't automatically list it in the **Explorer** pane.

To grant an IAM role access to a dataset without using
conditions, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Sharing**
   \> **Permissions**.

5. Click **Add principal**.

6. In the **New principals** field, enter a principal.

7. In the **Select a role** list, select a predefined role or a custom role.

8. Click **Save**.

9. To return to the dataset info, click **Close**.

### SQL

To grant principals access to datasets, use the
[`GRANT` DCL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#grant_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   GRANT `ROLE_LIST`
   ON SCHEMA RESOURCE_NAME
   TO "USER_LIST"
   ```


   Replace the following:
   - `ROLE_LIST`: a role or list of comma-separated roles that you want to grant
   - `RESOURCE_NAME`: the name of the dataset that you're granting access to
   - `USER_LIST`: a comma-separated list of users
     that the role is granted to

     For a list of valid formats, see
     [`user_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#user_list).

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

The following example grants the BigQuery Data Viewer role to `myDataset`:

    GRANT `roles/bigquery.dataViewer`
    ON SCHEMA `myProject`.myDataset
    TO "user:user@example.com", "user:user2@example.com"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To write the existing dataset information (including access controls) to
   a JSON file, use the
   [`bq show` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show):

   ```bash
   bq show \
       --format=prettyjson \
       PROJECT_ID:DATASET > PATH_TO_FILE
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">DATASET</var>: the name of your dataset
   - <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine
3. Make changes to the `access` section of the JSON file. You can
   add to any of the `specialGroup` entries: `projectOwners`,
   `projectWriters`, `projectReaders`, and `allAuthenticatedUsers`. You can
   also add any of the following: `userByEmail`, `groupByEmail`, and
   `domain`.

   For example, the `access` section of a dataset's JSON file would look
   like the following:

   ```json
   {
    "access": [
     {
      "role": "READER",
      "specialGroup": "projectReaders"
     },
     {
      "role": "WRITER",
      "specialGroup": "projectWriters"
     },
     {
      "role": "OWNER",
      "specialGroup": "projectOwners"
     },
     {
      "role": "READER",
      "specialGroup": "allAuthenticatedUsers"
     },
     {
      "role": "READER",
      "domain": "domain_name"
     },
     {
      "role": "WRITER",
      "userByEmail": "user_email"
     },
     {
      "role": "READER",
      "groupByEmail": "group_email"
     }
    ],
    ...
   }
   ```
4. When your edits are complete, use the `bq update` command and include
   the JSON file using the `--source` flag. If the dataset is in a project
   other than your default project, add the project ID to the dataset name
   in the following format: `PROJECT_ID:DATASET`.

   > [!CAUTION]
   > **Caution:** When you apply the JSON file that contains the access controls, the existing access controls are overwritten.

   <br />

   ```bash
     bq update 

     --source PATH_TO_FILE 

     PROJECT_ID:DATASET
     
   ```

   <br />

5. To verify your access control changes, use the `bq show` command again
   without writing the information to a file:

   ```bash
   bq show --format=prettyjson PROJECT_ID:DATASET
   ```

### Terraform

Use the
[`google_bigquery_dataset_iam`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_dataset_iam) resources to update
access to a dataset.

> [!IMPORTANT]
> **Important:** The different resources provided by `google_bigquery_dataset_iam` can conflict with each other as well as with the [google_bigquery_dataset_access](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_dataset_access) resource. Read the [`google_bigquery_dataset_iam`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_dataset_iam) documentation carefully before making access control changes using Terraform.

**Set the access policy for a dataset**

The following example shows how to use the
[`google_bigquery_dataset_iam_policy` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_dataset_iam#google_bigquery_dataset_iam_policy)
to set the IAM policy for the
`mydataset` dataset. This replaces any existing policy already attached
to the dataset:

```bash
# This file sets the IAM policy for the dataset created by
# https://github.com/terraform-google-modules/terraform-docs-samples/blob/main/bigquery/bigquery_create_dataset/main.tf.
# You must place it in the same local directory as that main.tf file,
# and you must have already applied that main.tf file to create
# the "default" dataset resource with a dataset_id of "mydataset".

data "google_iam_policy" "iam_policy" {
  binding {
    role = "roles/bigquery.admin"
    members = [
      "user:user@example.com",
    ]
  }
  binding {
    role = "roles/bigquery.dataOwner"
    members = [
      "group:data.admin@example.com",
    ]
  }
  binding {
    role = "roles/bigquery.dataEditor"
    members = [
      "serviceAccount:bqcx-1234567891011-12a3@gcp-sa-bigquery-condel.iam.gserviceaccount.com",
    ]
  }
}

resource "google_bigquery_dataset_iam_policy" "dataset_iam_policy" {
  dataset_id  = google_bigquery_dataset.default.dataset_id
  policy_data = data.google_iam_policy.iam_policy.policy_data
}
```

**Set role membership for a dataset**

The following example shows how to use the
[`google_bigquery_dataset_iam_binding` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_dataset_iam#google_bigquery_dataset_iam_binding)
to set membership in a given role for the
`mydataset` dataset. This replaces any existing membership in that role.
Other roles within the IAM policy for the dataset
are preserved:

```bash
# This file sets membership in an IAM role for the dataset created by
# https://github.com/terraform-google-modules/terraform-docs-samples/blob/main/bigquery/bigquery_create_dataset/main.tf.
# You must place it in the same local directory as that main.tf file,
# and you must have already applied that main.tf file to create
# the "default" dataset resource with a dataset_id of "mydataset".

resource "google_bigquery_dataset_iam_binding" "dataset_iam_binding" {
  dataset_id = google_bigquery_dataset.default.dataset_id
  role       = "roles/bigquery.jobUser"

  members = [
    "user:user@example.com",
    "group:group@example.com"
  ]
}
```

**Set role membership for a single principal**

The following example shows how to use the
[`google_bigquery_dataset_iam_member` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_dataset_iam#google_bigquery_dataset_iam_member)
to update the IAM policy for the
`mydataset` dataset to grant a role to one principal. Updating this
IAM policy does not affect access for any other principals
that have been granted that role for the dataset.

```bash
# This file adds a member to an IAM role for the dataset created by
# https://github.com/terraform-google-modules/terraform-docs-samples/blob/main/bigquery/bigquery_create_dataset/main.tf.
# You must place it in the same local directory as that main.tf file,
# and you must have already applied that main.tf file to create
# the "default" dataset resource with a dataset_id of "mydataset".

resource "google_bigquery_dataset_iam_member" "dataset_iam_member" {
  dataset_id = google_bigquery_dataset.default.dataset_id
  role       = "roles/bigquery.user"
  member     = "user:user@example.com"
}
```

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

### API

To apply access controls when the dataset is created, call the
[`datasets.insert` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/insert)
with a defined
[dataset resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).
To update your access controls, call the
[`datasets.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch) and
use the `access` property in the `Dataset` resource.

Because the `datasets.update` method replaces the entire dataset resource,
`datasets.patch` is the preferred method for updating access controls.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Set the new access list by appending the new entry to the existing list with [`DatasetMetadataToUpdate` type](https://pkg.go.dev/cloud.google.com/go/bigquery#DatasetMetadataToUpdate) . Then call the [`dataset.Update()` function](https://pkg.go.dev/cloud.google.com/go/bigquery#Dataset.Update) to update the property.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    )

    // grantAccessToDataset creates a new ACL conceding the READER role to the group "example-analyst-group@google.com"
    // For more information on the types of ACLs available see:
    // https://cloud.google.com/storage/docs/access-control/lists
    func grantAccessToDataset(w io.Writer, projectID, datasetID string) error {
    	// TODO(developer): uncomment and update the following lines:
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"

    	ctx := context.Background()

    	// Create BigQuery handler.
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	// Create dataset handler
    	dataset := client.Dataset(datasetID)

    	// Get metadata
    	meta, err := dataset.Metadata(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Metadata: %w", err)
    	}

    	// Find more details about BigQuery Entity Types here:
    	// https://pkg.go.dev/cloud.google.com/go/bigquery#EntityType
    	//
    	// Find more details about BigQuery Access Roles here:
    	// https://pkg.go.dev/cloud.google.com/go/bigquery#AccessRole

    	entityType := bigquery.GroupEmailEntity
    	entityID := "example-analyst-group@google.com"
    	roleType := bigquery.ReaderRole

    	// Append a new access control entry to the existing access list.
    	update := bigquery.DatasetMetadataToUpdate{
    		Access: append(meta.Access, &bigquery.AccessEntry{
    			Role:       roleType,
    			EntityType: entityType,
    			Entity:     entityID,
    		}),
    	}

    	// Leverage the ETag for the update to assert there's been no modifications to the
    	// dataset since the metadata was originally read.
    	meta, err = dataset.Update(ctx, update, meta.ETag)
    	if err != nil {
    		return err
    	}

    	fmt.Fprintf(w, "Details for Access entries in dataset %v.\n", datasetID)
    	for _, access := range meta.Access {
    		fmt.Fprintln(w)
    		fmt.Fprintf(w, "Role: %s\n", access.Role)
    		fmt.Fprintf(w, "Entities: %v\n", access.Entity)
    	}

    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.bigquery.Acl;
    import com.google.cloud.bigquery.Acl.Entity;
    import com.google.cloud.bigquery.Acl.Group;
    import com.google.cloud.bigquery.Acl.Role;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryException;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.Dataset;
    import com.google.cloud.bigquery.DatasetId;
    import java.util.ArrayList;
    import java.util.List;

    public class GrantAccessToDataset {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        // Project and dataset from which to get the access policy
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        // Group to add to the ACL
        String entityEmail = "group-to-add@example.com";

        grantAccessToDataset(projectId, datasetName, entityEmail);
      }

      public static void grantAccessToDataset(
          String projectId, String datasetName, String entityEmail) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Create datasetId with the projectId and the datasetName.
          DatasetId datasetId = DatasetId.of(projectId, datasetName);
          Dataset dataset = bigquery.getDataset(datasetId);

          // Create a new Entity with the corresponding type and email
          // "user-or-group-to-add@example.com"
          // For more information on the types of Entities available see:
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Acl.Entity
          // and
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Acl.Entity.Type
          Entity entity = new Group(entityEmail);

          // Create a new ACL granting the READER role to the group with the entity email
          // "user-or-group-to-add@example.com"
          // For more information on the types of ACLs available see:
          // https://cloud.google.com/storage/docs/access-control/lists
          Acl newEntry = Acl.of(entity, Role.READER);

          // Get a copy of the ACLs list from the dataset and append the new entry.
          List<Acl> acls = new ArrayList<>(dataset.getAcl());
          acls.add(newEntry);

          // Update the ACLs by setting the new list.
          Dataset updatedDataset = bigquery.update(dataset.toBuilder().setAcl(acls).build());
          System.out.println(
              "ACLs of dataset \""
                  + updatedDataset.getDatasetId().getDataset()
                  + "\" updated successfully");
        } catch (BigQueryException e) {
          System.out.println("ACLs were not updated \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Set the new access list by appending the new entry to the existing list using the [Dataset#metadata](https://googleapis.dev/nodejs/bigquery/latest/Dataset.html#metadata) method. Then call the [Dataset#setMetadata()](https://googleapis.dev/nodejs/bigquery/latest/Dataset.html#setMetadata) function to update the property.


    /**
     * TODO(developer): Update and un-comment below lines.
     */

    // const datasetId = "my_project_id.my_dataset_name";

    // ID of the user or group from whom you are adding access.
    // const entityId = "user-or-group-to-add@example.com";

    // One of the "Basic roles for datasets" described here:
    // https://cloud.google.com/bigquery/docs/access-control-basic-roles#dataset-basic-roles
    // const role = "READER";

    const {BigQuery} = require('@google-cloud/bigquery');

    // Instantiate a client.
    const client = new BigQuery();

    // Type of entity you are granting access to.
    // Find allowed allowed entity type names here:
    // https://cloud.google.com/bigquery/docs/reference/rest/v2/datasets#resource:-dataset
    const entityType = 'groupByEmail';

    async function grantAccessToDataset() {
      const [dataset] = await client.dataset(datasetId).get();

      // The 'access entries' array is immutable. Create a copy for modifications.
      const entries = [...dataset.metadata.access];

      // Append an AccessEntry to grant the role to a dataset.
      // Find more details about the AccessEntry object in the BigQuery documentation:
      // https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry
      entries.push({
        role,
        [entityType]: entityId,
      });

      // Assign the array of AccessEntries back to the dataset.
      const metadata = {
        access: entries,
      };

      // Update will only succeed if the dataset
      // has not been modified externally since retrieval.
      //
      // See the BigQuery client library documentation for more details on metadata updates:
      // https://cloud.google.com/nodejs/docs/reference/bigquery/latest

      // Update just the 'access entries' property of the dataset.
      await client.dataset(datasetId).setMetadata(metadata);

      console.log(
        `Role '${role}' granted for entity '${entityId}' in '${datasetId}'.`
      );
    }

<br />

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Set the [`dataset.access_entries` property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.Dataset#google_cloud_bigquery_dataset_Dataset_access_entries) with the access controls for a dataset. Then call the [`client.update_dataset()` function](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_update_dataset) to update the property.

    from google.api_core.exceptions import PreconditionFailed
    from google.cloud import bigquery
    from google.cloud.bigquery.enums import EntityTypes

    # TODO(developer): Update and uncomment the lines below.

    # ID of the dataset to grant access to.
    # dataset_id = "my_project_id.my_dataset"

    # ID of the user or group receiving access to the dataset.
    # Alternatively, the JSON REST API representation of the entity,
    # such as the view's table reference.
    # entity_id = "user-or-group-to-add@example.com"

    # One of the "Basic roles for datasets" described here:
    # https://cloud.google.com/bigquery/docs/access-control-basic-roles#dataset-basic-roles
    # role = "READER"

    # Type of entity you are granting access to.
    # Find allowed allowed entity type names here:
    # https://cloud.google.com/python/docs/reference/bigquery/latest/enums#class-googlecloudbigqueryenumsentitytypesvalue
    entity_type = EntityTypes.GROUP_BY_EMAIL

    # Instantiate a client.
    client = bigquery.Client()

    # Get a reference to the dataset.
    dataset = client.get_dataset(dataset_id)

    # The `access_entries` list is immutable. Create a copy for modifications.
    entries = list(dataset.access_entries)

    # Append an AccessEntry to grant the role to a dataset.
    # Find more details about the AccessEntry object here:
    # https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry
    entries.append(
        bigquery.AccessEntry(
            role=role,
            entity_type=entity_type,
            entity_id=entity_id,
        )
    )

    # Assign the list of AccessEntries back to the dataset.
    dataset.access_entries = entries

    # Update will only succeed if the dataset
    # has not been modified externally since retrieval.
    #
    # See the BigQuery client library documentation for more details on `update_dataset`:
    # https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_update_dataset
    try:
        # Update just the `access_entries` property of the dataset.
        dataset = client.update_dataset(
            dataset,
            ["access_entries"],
        )

        # Show a success message.
        full_dataset_id = f"{dataset.project}.{dataset.dataset_id}"
        print(
            f"Role '{role}' granted for entity '{entity_id}'"
            f" in dataset '{full_dataset_id}'."
        )
    except PreconditionFailed:  # A read-modify-write error
        print(
            f"Dataset '{dataset.dataset_id}' was modified remotely before this update. "
            "Fetch the latest version and retry."
        )

<br />

### Predefined roles that grant access to datasets

You can grant the following IAM predefined roles access to a
dataset.

> [!CAUTION]
> **Caution:** While it is possible to grant BigQuery Admin or BigQuery Studio Admin permissions on a dataset, you shouldn't grant these roles at the dataset level. BigQuery Data Owner also grants all permissions for the dataset and is a less permissive role. BigQuery Admin and BigQuery Studio Admin are typically granted at the project level.

| Role | Description |
|---|---|
| [BigQuery Data Owner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner) (`roles/bigquery.dataOwner`) | When granted on a dataset, this role grants these permissions: - All permissions for the dataset and for all of the resources within the dataset: tables and views, models, and routines. > [!NOTE] > **Note:** Principals that are granted the Data Owner role at the project level can also create new datasets and list datasets in the project that they have access to. |
| [BigQuery Data Editor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor) (`roles/bigquery.dataEditor`) | When granted on a dataset, this role grants these permissions: - Get metadata and permissions for the dataset. - For tables and views: - Create, update, get, list, and delete the dataset's tables and views. - Read (query), export, replicate, and update table data. - Create, update, and delete indexes. - Create and restore snapshots. - All permissions for the dataset's routines and models. > [!NOTE] > **Note:** Principals that are granted the Data Editor role at the project level can also create new datasets and list datasets in the project that they have access to. |
| [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer) (`roles/bigquery.dataViewer`) | When granted on a dataset, this role grants these permissions: - Get metadata and permissions for the dataset. - List a dataset's tables, views, and models. - Get metadata and access controls for the dataset's tables and views. - Read (query), replicate, and export table data and create snapshots. - List and invoke the dataset's routines. |
| [BigQuery Metadata Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.metadataViewer) (`roles/bigquery.metadataViewer`) | When granted on a dataset, this role grants these permissions: - Get metadata and access controls for the dataset. - Get metadata and access controls for tables and views. - Get metadata from the dataset's models and routines. - List tables, views, models, and routines in the dataset. |

### Dataset permissions

Most permissions that begin with `bigquery.datasets` apply at the dataset level.
`bigquery.datasets.create` doesn't. In order to create datasets,
`bigquery.datasets.create` permission must be granted to a role on the parent
container--the project.

The following table lists all permissions for datasets and the lowest-level
resource the permission can be applied to.

| Permission | Resource | Action |
|---|---|---|
| `bigquery.datasets.create` | Project | Create new datasets in the project. |
| `bigquery.datasets.get` | Dataset | Get metadata and access controls for the dataset. Viewing permissions in the console also requires the `bigquery.datasets.getIamPolicy` permission. |
| `bigquery.datasets.getIamPolicy` | Dataset | Required by the console to grant the user permission to get a dataset's access controls. Fails open. The console also requires the `bigquery.datasets.get` permission to view the dataset. |
| `bigquery.datasets.update` | Dataset | Update metadata and access controls for the dataset. Updating access controls in the console also requires the `bigquery.datasets.setIamPolicy` permission. |
| `bigquery.datasets.setIamPolicy` | Dataset | Required by the console to grant the user permission to set a dataset's access controls. Fails open. The console also requires the `bigquery.datasets.update` permission to update the dataset. |
| `bigquery.datasets.delete` | Dataset | Delete a dataset. |
| `bigquery.datasets.createTagBinding` | Dataset | Attach tags to the dataset. |
| `bigquery.datasets.deleteTagBinding` | Dataset | Detach tags from the dataset. |
| `bigquery.datasets.listTagBindings` | Dataset | List tags for the dataset. |
| `bigquery.datasets.listEffectiveTags` | Dataset | List effective tags (applied and inherited) for the dataset. |
| `bigquery.datasets.link` | Dataset | Create a [linked dataset](https://docs.cloud.google.com/logging/docs/analyze/query-linked-dataset). |
| `bigquery.datasets.listSharedDatasetUsage` | Project | List shared dataset usage statistics for datasets that you have access to in the project. This permission is required to query the `INFORMATION_SCHEMA.SHARED_DATASET_USAGE` view. |

### View access controls for a dataset

You can view the explicitly set access controls for a dataset by choosing one of
the following options. To [view inherited roles](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#inherited-roles), for a
dataset, use the BigQuery web UI.

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Sharing \> Permissions**.

   The dataset's access controls appear in the **Dataset Permissions** pane.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To get an existing policy and output it to a local file in JSON, use the
   [`bq show` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show)
   in Cloud Shell:

   ```bash
   bq show \
      --format=prettyjson \
      PROJECT_ID:DATASET > PATH_TO_FILE
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">DATASET</var>: the name of your dataset
   - <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine

### SQL


> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

<br />

Query the [`INFORMATION_SCHEMA.OBJECT_PRIVILEGES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges).
Queries to retrieve access controls for a dataset must specify the
`object_name`.


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
   COLUMN_LIST
   FROM
     PROJECT_ID.`region-REGION`.INFORMATION_SCHEMA.OBJECT_PRIVILEGES
   WHERE
   object_name = "DATASET";
   ```


   Replace the following:
   - <var translate="no">COLUMN_LIST</var>: a comma-separated list of columns from the [`INFORMATION_SCHEMA.OBJECT_PRIVILEGES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">REGION</var>: a [region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier)
   - <var translate="no">DATASET</var>: the name of a dataset in your project

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Example:

This query gets access controls for `mydataset`.

```googlesql
SELECT
object_name, privilege_type, grantee
FROM
my_project.`region-us`.INFORMATION_SCHEMA.OBJECT_PRIVILEGES
WHERE
object_name = "mydataset";
```

The output should look like the following:

    +---+---+---+
    |   object_name    |  privilege_type             | grantee                 |
    +---+---+---+
    | mydataset        | roles/bigquery.dataOwner    | projectOwner:myproject  |
    | mydataset        | roles/bigquery.dataViwer    | user:user@example.com   |
    +---+---+---+

### API

To view the access controls for a dataset, call the
[`datasets.get`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/get)
method with a defined
[`dataset` resource](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets).

The access controls appear in the `access` property of the `dataset` resource.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the [`client.Dataset().Metadata()` function](https://pkg.go.dev/cloud.google.com/go/bigquery#Dataset.Metadata). The access policy is available in the [`Access`](https://pkg.go.dev/cloud.google.com/go/bigquery@v1.66.0#DatasetMetadata.Access) property.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    )

    // viewDatasetAccessPolicies retrieves the ACL for the given dataset
    // For more information on the types of ACLs available see:
    // https://cloud.google.com/storage/docs/access-control/lists
    func viewDatasetAccessPolicies(w io.Writer, projectID, datasetID string) error {
    	// TODO(developer): uncomment and update the following lines:
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"

    	ctx := context.Background()

    	// Create new client.
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	// Get dataset's metadata.
    	meta, err := client.Dataset(datasetID).Metadata(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Client.Dataset.Metadata: %w", err)
    	}

    	fmt.Fprintf(w, "Details for Access entries in dataset %v.\n", datasetID)

    	// Iterate over access permissions.
    	for _, access := range meta.Access {
    		fmt.Fprintln(w)
    		fmt.Fprintf(w, "Role: %s\n", access.Role)
    		fmt.Fprintf(w, "Entity: %v\n", access.Entity)
    	}

    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    import com.google.cloud.bigquery.Acl;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryException;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.Dataset;
    import com.google.cloud.bigquery.DatasetId;
    import java.util.List;

    public class GetDatasetAccessPolicy {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        // Project and dataset from which to get the access policy.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        getDatasetAccessPolicy(projectId, datasetName);
      }

      public static void getDatasetAccessPolicy(String projectId, String datasetName) {
        try {
          // Initialize client that will be used to send requests. This client only needs to be created
          // once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Create datasetId with the projectId and the datasetName.
          DatasetId datasetId = DatasetId.of(projectId, datasetName);
          Dataset dataset = bigquery.getDataset(datasetId);

          // Show ACL details.
          // Find more information about ACL and the Acl Class here:
          // https://cloud.google.com/storage/docs/access-control/lists
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Acl
          List<Acl> acls = dataset.getAcl();
          System.out.println("ACLs in dataset \"" + dataset.getDatasetId().getDataset() + "\":");
          System.out.println(acls.toString());
          for (Acl acl : acls) {
            System.out.println();
            System.out.println("Role: " + acl.getRole());
            System.out.println("Entity: " + acl.getEntity());
          }
        } catch (BigQueryException e) {
          System.out.println("ACLs info not retrieved. \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Retrieve the dataset metadata using the [`Dataset#getMetadata()` function](https://googleapis.dev/nodejs/bigquery/latest/Dataset.html#getMetadata). The access policy is available in the access property of the resulting metadata object.


    /**
     * TODO(developer): Update and un-comment below lines
     */
    // const datasetId = "my_project_id.my_dataset";

    const {BigQuery} = require('@google-cloud/bigquery');

    // Instantiate a client.
    const bigquery = new BigQuery();

    async function viewDatasetAccessPolicy() {
      const dataset = bigquery.dataset(datasetId);

      const [metadata] = await dataset.getMetadata();
      const accessEntries = metadata.access || [];

      // Show the list of AccessEntry objects.
      // More details about the AccessEntry object in the BigQuery documentation:
      // https://cloud.google.com/nodejs/docs/reference/bigquery/latest
      console.log(
        `${accessEntries.length} Access entries in dataset '${datasetId}':`
      );
      for (const accessEntry of accessEntries) {
        console.log(`Role: ${accessEntry.role || 'null'}`);
        console.log(`Special group: ${accessEntry.specialGroup || 'null'}`);
        console.log(`User by Email: ${accessEntry.userByEmail || 'null'}`);
      }
    }

<br />

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the [`client.get_dataset()` function](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_get_dataset). The access policy is available in the [`dataset.access_entries` property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.Dataset#google_cloud_bigquery_dataset_Dataset_access_entries).

    from google.cloud import bigquery

    # Instantiate a client.
    client = bigquery.Client()

    # TODO(developer): Update and uncomment the lines below.

    # Dataset from which to get the access policy.
    # dataset_id = "my_dataset"

    # Get a reference to the dataset.
    dataset = client.get_dataset(dataset_id)

    # Show the list of AccessEntry objects.
    # More details about the AccessEntry object here:
    # https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.AccessEntry
    print(
        f"{len(dataset.access_entries)} Access entries found "
        f"in dataset '{dataset_id}':"
    )

    for access_entry in dataset.access_entries:
        print()
        print(f"Role: {access_entry.role}")
        print(f"Special group: {access_entry.special_group}")
        print(f"User by Email: {access_entry.user_by_email}")

<br />

### Revoke access to a dataset

To revoke access to a dataset, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. In the details panel, click **Sharing \> Permissions**.

5. In the **Dataset Permissions** dialog, expand the principal whose access
   you want to revoke.

6. Click **Remove principal**.

7. In the **Remove role from principal?** dialog, click **Remove**.

8. To return to dataset details, click **Close**.

> [!NOTE]
> **Note:** If you cannot revoke access to the dataset, the principal may have [inherited access](https://docs.cloud.google.com/bigquery/docs/control-access-to-resources-iam#inherited-roles) from a higher level in the [resource hierarchy](https://docs.cloud.google.com/iam/docs/overview#resource-hierarchy).

### SQL

To remove a principal's access to a dataset, use the
[`REVOKE` DCL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#revoke_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   REVOKE `ROLE_LIST`
   ON SCHEMA RESOURCE_NAME
   FROM "USER_LIST"
   ```


   Replace the following:
   - `ROLE_LIST`: a role or list of comma-separated roles that you want to revoke
   - `RESOURCE_NAME`: the name of the resource that you want to revoke permission on
   - `USER_LIST`: a comma-separated list of users
     who will have their roles revoked

     For a list of valid formats, see
     [`user_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#user_list).

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

The following example revokes the BigQuery Data Owner role from `myDataset`:

    REVOKE `roles/bigquery.dataOwner`
    ON SCHEMA `myProject`.myDataset
    FROM "group:group@example.com", "serviceAccount:user@test-project.iam.gserviceaccount.com"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To write the existing dataset information (including access controls) to
   a JSON file, use the
   [`bq show` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_show):

   ```bash
   bq show \
       --format=prettyjson \
       PROJECT_ID:DATASET > PATH_TO_FILE
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">DATASET</var>: the name of your dataset
   - <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine
3. Make changes to the `access` section of the JSON file. You can
   remove any of the `specialGroup` entries: `projectOwners`,
   `projectWriters`, `projectReaders`, and `allAuthenticatedUsers`. You can
   also remove any of the following: `userByEmail`,
   `groupByEmail`, and `domain`.

   For example, the `access` section of a dataset's JSON file would look
   like the following:

   ```json
   {
    "access": [
     {
      "role": "READER",
      "specialGroup": "projectReaders"
     },
     {
      "role": "WRITER",
      "specialGroup": "projectWriters"
     },
     {
      "role": "OWNER",
      "specialGroup": "projectOwners"
     },
     {
      "role": "READER",
      "specialGroup": "allAuthenticatedUsers"
     },
     {
      "role": "READER",
      "domain": "domain_name"
     },
     {
      "role": "WRITER",
      "userByEmail": "user_email"
     },
     {
      "role": "READER",
      "groupByEmail": "group_email"
     }
    ],
    ...
   }
   ```
4. When your edits are complete, use the `bq update` command and include
   the JSON file using the `--source` flag. If the dataset is in a project
   other than your default project, add the project ID to the dataset name
   in the following format: `PROJECT_ID:DATASET`.

   > [!CAUTION]
   > **Caution:** When you apply the JSON file that contains the access controls, the existing access controls are overwritten.

   <br />

   ```bash
     bq update 

         --source PATH_TO_FILE 

         PROJECT_ID:DATASET
     
   ```

   <br />

5. To verify your access control changes, use the `show` command without
   writing the information to a file:

   ```bash
   bq show --format=prettyjson PROJECT_ID:DATASET
   ```

### API

Call the [`datasets.patch` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/datasets/patch)
and use the `access` property in the `Dataset` resource to update your access
controls.

Because the `datasets.update` method replaces the entire dataset resource,
`datasets.patch` is the preferred method for updating access controls.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Set the new access list by removing the entry from the existing list with [`DatasetMetadataToUpdate` type](https://pkg.go.dev/cloud.google.com/go/bigquery#DatasetMetadataToUpdate) . Then call the [`dataset.Update()` function](https://pkg.go.dev/cloud.google.com/go/bigquery#Dataset.Update) to update the property.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    )

    // revokeAccessToDataset creates a new ACL removing the dataset access to "example-analyst-group@google.com" entity
    // For more information on the types of ACLs available see:
    // https://cloud.google.com/storage/docs/access-control/lists
    func revokeAccessToDataset(w io.Writer, projectID, datasetID, entity string) error {
    	// TODO(developer): uncomment and update the following lines:
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// entity := "user@mydomain.com"

    	ctx := context.Background()

    	// Create BigQuery client.
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	// Get dataset handler
    	dataset := client.Dataset(datasetID)

    	// Get dataset metadata
    	meta, err := dataset.Metadata(ctx)
    	if err != nil {
    		return err
    	}

    	// Create new access entry list by copying the existing and omiting the access entry entity value
    	var newAccessList []*bigquery.AccessEntry
    	for _, entry := range meta.Access {
    		if entry.Entity != entity {
    			newAccessList = append(newAccessList, entry)
    		}
    	}

    	// Only proceed with update if something in the access list was removed.
    	// Additionally, we use the ETag from the initial metadata to ensure no
    	// other changes were made to the access list in the interim.
    	if len(newAccessList) < len(meta.Access) {
    		update := bigquery.DatasetMetadataToUpdate{
    			Access: newAccessList,
    		}
    		meta, err = dataset.Update(ctx, update, meta.ETag)
    		if err != nil {
    			return err
    		}
    	} else {
    		return fmt.Errorf("any access entry was revoked")
    	}

    	fmt.Fprintf(w, "Details for Access entries in dataset %v.\n", datasetID)

    	for _, access := range meta.Access {
    		fmt.Fprintln(w)
    		fmt.Fprintf(w, "Role: %s\n", access.Role)
    		fmt.Fprintf(w, "Entity: %v\n", access.Entity)
    	}

    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    import com.google.cloud.bigquery.Acl;
    import com.google.cloud.bigquery.Acl.Entity;
    import com.google.cloud.bigquery.Acl.Group;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryException;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.Dataset;
    import com.google.cloud.bigquery.DatasetId;
    import java.util.List;

    public class RevokeDatasetAccess {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        // Project and dataset from which to get the access policy.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        // Group to remove from the ACL
        String entityEmail = "group-to-remove@example.com";

        revokeDatasetAccess(projectId, datasetName, entityEmail);
      }

      public static void revokeDatasetAccess(String projectId, String datasetName, String entityEmail) {
        try {
          // Initialize client that will be used to send requests. This client only needs
          // to be created once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Create datasetId with the projectId and the datasetName.
          DatasetId datasetId = DatasetId.of(projectId, datasetName);
          Dataset dataset = bigquery.getDataset(datasetId);

          // Create a new Entity with the corresponding type and email
          // "user-or-group-to-remove@example.com"
          // For more information on the types of Entities available see:
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Acl.Entity
          // and
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Acl.Entity.Type
          Entity entity = new Group(entityEmail);

          // To revoke access to a dataset, remove elements from the Acl list.
          // Find more information about ACL and the Acl Class here:
          // https://cloud.google.com/storage/docs/access-control/lists
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/com.google.cloud.bigquery.Acl
          // Remove the entity from the ACLs list.
          List<Acl> acls =
              dataset.getAcl().stream().filter(acl -> !acl.getEntity().equals(entity)).toList();

          // Update the ACLs by setting the new list.
          bigquery.update(dataset.toBuilder().setAcl(acls).build());
          System.out.println("ACLs of \"" + datasetName + "\" updated successfully");
        } catch (BigQueryException e) {
          System.out.println("ACLs were not updated \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Update the dataset access list by removing the specified entry from the existing list using the [`Dataset#get()`](https://googleapis.dev/nodejs/bigquery/latest/Dataset.html#get) method to retrieve the current metadata. Modify the access property to exclude the desired entity, and then call the [`Dataset#setMetadata()`](https://googleapis.dev/nodejs/bigquery/latest/Dataset.html#setMetadata) function to apply the updated access list.


    /**
     * TODO(developer): Update and un-comment below lines
     */

    // const datasetId = "my_project_id.my_dataset"

    // ID of the user or group from whom you are revoking access.
    // const entityId = "user-or-group-to-remove@example.com"

    const {BigQuery} = require('@google-cloud/bigquery');

    // Instantiate a client.
    const bigquery = new BigQuery();

    async function revokeDatasetAccess() {
      const [dataset] = await bigquery.dataset(datasetId).get();

      // To revoke access to a dataset, remove elements from the access list.
      //
      // See the BigQuery client library documentation for more details on access entries:
      // https://cloud.google.com/nodejs/docs/reference/bigquery/latest

      // Filter access entries to exclude entries matching the specified entity_id
      // and assign a new list back to the access list.
      dataset.metadata.access = dataset.metadata.access.filter(entry => {
        return !(
          entry.entity_id === entityId ||
          entry.userByEmail === entityId ||
          entry.groupByEmail === entityId
        );
      });

      // Update will only succeed if the dataset
      // has not been modified externally since retrieval.
      //
      // See the BigQuery client library documentation for more details on metadata updates:
      // https://cloud.google.com/bigquery/docs/updating-datasets

      // Update just the 'access entries' property of the dataset.
      await dataset.setMetadata(dataset.metadata);

      console.log(`Revoked access to '${entityId}' from '${datasetId}'.`);
    }

<br />

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Set the [`dataset.access_entries` property](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.Dataset#google_cloud_bigquery_dataset_Dataset_access_entries) with the access controls for a dataset. Then call the [`client.update_dataset()` function](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_update_dataset) to update the property.

    from google.cloud import bigquery
    from google.api_core.exceptions import PreconditionFailed

    # TODO(developer): Update and uncomment the lines below.

    # ID of the dataset to revoke access to.
    # dataset_id = "my-project.my_dataset"

    # ID of the user or group from whom you are revoking access.
    # Alternatively, the JSON REST API representation of the entity,
    # such as a view's table reference.
    # entity_id = "user-or-group-to-remove@example.com"

    # Instantiate a client.
    client = bigquery.Client()

    # Get a reference to the dataset.
    dataset = client.get_dataset(dataset_id)

    # To revoke access to a dataset, remove elements from the AccessEntry list.
    #
    # See the BigQuery client library documentation for more details on `access_entries`:
    # https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.dataset.Dataset#google_cloud_bigquery_dataset_Dataset_access_entries

    # Filter `access_entries` to exclude entries matching the specified entity_id
    # and assign a new list back to the AccessEntry list.
    dataset.access_entries = [
        entry for entry in dataset.access_entries
        if entry.entity_id != entity_id
    ]

    # Update will only succeed if the dataset
    # has not been modified externally since retrieval.
    #
    # See the BigQuery client library documentation for more details on `update_dataset`:
    # https://cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_update_dataset
    try:
        # Update just the `access_entries` property of the dataset.
        dataset = client.update_dataset(
            dataset,
            ["access_entries"],
        )

        # Notify user that the API call was successful.
        full_dataset_id = f"{dataset.project}.{dataset.dataset_id}"
        print(f"Revoked dataset access for '{entity_id}' to ' dataset '{full_dataset_id}.'")
    except PreconditionFailed:  # A read-modify-write error.
        print(
            f"Dataset '{dataset.dataset_id}' was modified remotely before this update. "
            "Fetch the latest version and retry."
        )

<br />

## Work with table and view access controls

Views are treated as table resources in BigQuery. You can provide
access to a table or view by granting an [IAM principal](https://docs.cloud.google.com/iam/docs/principal-identifiers#allow)
a predefined or custom role that determines what the principal can do with the
table or view. This is also known as attaching an [allow policy](https://docs.cloud.google.com/iam/docs/allow-policies)
to a resource. After granting access, you can view the access controls for the
table or view, and you can revoke access to the table or view.

### Grant access to a table or view

For fine-grained access control, you can grant a predefined or custom
IAM role on a specific table or view. The table or view also
inherits access controls specified at the dataset level and higher. For example,
if you grant a principal the BigQuery Data Owner role on a dataset, that
principal also has BigQuery Data Owner permissions on the tables and views in
the dataset.

To grant access to a table or view, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then click a table or a view.

5. Click **Share \> Manage permissions**.

6. Click **Add principal**.

7. In the **New principals** field, enter a principal.

8. In the **Select a role** list, select a predefined role or a custom role.

9. Click **Save**.

10. To return to the table or view details, click **Close**.

### SQL

To grant principals access to tables or views, use the
[`GRANT` DCL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#grant_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   GRANT `ROLE_LIST`
   ON RESOURCE_TYPE RESOURCE_NAME
   TO "USER_LIST"
   ```


   Replace the following:
   - `ROLE_LIST`: a role or list of comma-separated roles that you want to grant
   - `RESOURCE_TYPE`: the type of resource
     that the role is applied to

     Supported values include
     `TABLE`, `VIEW`, `MATERIALIZED
     VIEW` and `EXTERNAL TABLE`.
   - `RESOURCE_NAME`: the name of the resource that you want to grant the permission on
   - `USER_LIST`: a comma-separated list of users
     that the role is granted to

     For a list of valid formats, see
     [`user_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#user_list).

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

The following example grants the BigQuery Data Viewer role on `myTable`:

    GRANT `roles/bigquery.dataViewer`
    ON TABLE `myProject`.myDataset.myTable
    TO "user:user@example.com", "user:user2@example.com"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To grant access to a table or view, use the
   [`bq add-iam-policy-binding` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_add-iam-policy-binding):

   ```bash
   bq add-iam-policy-binding --member=MEMBER_TYPE:MEMBER --role=ROLE
     --table=true RESOURCE
   ```

   Replace the following:
   - <var translate="no">MEMBER_TYPE</var>: the type of member, such as `user`, `group`, `serviceAccount`, or `domain`.
   - <var translate="no">MEMBER</var>: the member's email address or domain name.
   - <var translate="no">ROLE</var>: the role that you want to grant to the member.
   - <var translate="no">RESOURCE</var>: the name of the table or view whose policy you want to update.

### Terraform

Use the
[`google_bigquery_table_iam`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table_iam) resources to update access to a table.

> [!IMPORTANT]
> **Important:** The different resources provided by `google_bigquery_table_iam` can conflict with each other. We recommend reading the [`google_bigquery_table_iam`](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table_iam) documentation carefully before making access control changes by using Terraform.

**Set the access policy for a table**

The following example shows how to use the
[`google_bigquery_table_iam_policy` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table_iam#google_bigquery_table_iam_policy)
to set the IAM policy for the
`mytable` table. This replaces any existing policy already attached
to the table:

```bash
# This file sets the IAM policy for the table created by
# https://github.com/terraform-google-modules/terraform-docs-samples/blob/main/bigquery/bigquery_create_table/main.tf.
# You must place it in the same local directory as that main.tf file,
# and you must have already applied that main.tf file to create
# the "default" table resource with a table_id of "mytable".

data "google_iam_policy" "iam_policy" {
  binding {
    role = "roles/bigquery.dataOwner"
    members = [
      "user:user@example.com",
    ]
  }
}

resource "google_bigquery_table_iam_policy" "table_iam_policy" {
  dataset_id  = google_bigquery_table.default.dataset_id
  table_id    = google_bigquery_table.default.table_id
  policy_data = data.google_iam_policy.iam_policy.policy_data
}
```

**Set role membership for a table**

The following example shows how to use the
[`google_bigquery_table_iam_binding` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table_iam#google_bigquery_table_iam_binding)
to set membership in a given role for the
`mytable` table. This replaces any existing membership in that role.
Other roles within the IAM policy for the table
are preserved.

```bash
# This file sets membership in an IAM role for the table created by
# https://github.com/terraform-google-modules/terraform-docs-samples/blob/main/bigquery/bigquery_create_table/main.tf.
# You must place it in the same local directory as that main.tf file,
# and you must have already applied that main.tf file to create
# the "default" table resource with a table_id of "mytable".

resource "google_bigquery_table_iam_binding" "table_iam_binding" {
  dataset_id = google_bigquery_table.default.dataset_id
  table_id   = google_bigquery_table.default.table_id
  role       = "roles/bigquery.dataOwner"

  members = [
    "group:group@example.com",
  ]
}
```

**Set role membership for a single principal**

The following example shows how to use the
[`google_bigquery_table_iam_member` resource](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/bigquery_table_iam#google_bigquery_table_iam_member)
to update the IAM policy for the
`mytable` table to grant a role to one principal. Updating this
IAM policy does not affect access for any other principals
that have been granted that role for the dataset.

```bash
# This file adds a member to an IAM role for the table created by
# https://github.com/terraform-google-modules/terraform-docs-samples/blob/main/bigquery/bigquery_create_table/main.tf.
# You must place it in the same local directory as that main.tf file,
# and you must have already applied that main.tf file to create
# the "default" table resource with a table_id of "mytable".

resource "google_bigquery_table_iam_member" "table_iam_member" {
  dataset_id = google_bigquery_table.default.dataset_id
  table_id   = google_bigquery_table.default.table_id
  role       = "roles/bigquery.dataEditor"
  member     = "serviceAccount:bqcx-1234567891011-12a3@gcp-sa-bigquery-condel.iam.gserviceaccount.com"
}
```

To apply your Terraform configuration in a Google Cloud project, complete the steps in the
following sections.

## Prepare Cloud Shell

1. Launch [Cloud Shell](https://shell.cloud.google.com/).
2. Set the default Google Cloud project
   where you want to apply your Terraform configurations.

   You only need to run this command once per project, and you can run it in any directory.

   ```
   export GOOGLE_CLOUD_PROJECT=PROJECT_ID
   ```

   Environment variables are overridden if you set explicit values in the Terraform
   configuration file.

## Prepare the directory

Each Terraform configuration file must have its own directory (also
called a *root module*).

1. In [Cloud Shell](https://shell.cloud.google.com/), create a directory and a new file within that directory. The filename must have the `.tf` extension---for example `main.tf`. In this tutorial, the file is referred to as `main.tf`.

   ```
   mkdir DIRECTORY && cd DIRECTORY && touch main.tf
   ```
2. If you are following a tutorial, you can copy the sample code in each section or step.

   Copy the sample code into the newly created `main.tf`.

   Optionally, copy the code from GitHub. This is recommended
   when the Terraform snippet is part of an end-to-end solution.
3. Review and modify the sample parameters to apply to your environment.
4. Save your changes.
5. Initialize Terraform. You only need to do this once per directory.

   ```
   terraform init
   ```

   Optionally, to use the latest Google provider version, include the `-upgrade`
   option:

   ```
   terraform init -upgrade
   ```

## Apply the changes

1. Review the configuration and verify that the resources that Terraform is going to create or update match your expectations:

   ```
   terraform plan
   ```

   Make corrections to the configuration as necessary.
2. Apply the Terraform configuration by running the following command and entering `yes` at the prompt:

   ```
   terraform apply
   ```

   Wait until Terraform displays the "Apply complete!" message.
3. [Open your Google Cloud project](https://console.cloud.google.com/) to view the results. In the Google Cloud console, navigate to your resources in the UI to make sure that Terraform has created or updated them.

> [!NOTE]
> **Note:** Terraform samples typically assume that the required APIs are enabled in your Google Cloud project.

### API

1. To retrieve the current policy, call the
   [`tables.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/getIamPolicy).

2. Edit the policy to add members or access controls, or both.
   For the format required for the policy, see the [Policy](https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy)
   reference topic.

   > [!CAUTION]
   > **Caution:** Before you set the policy, always retrieve the current policy to obtain the current value for `etag`. Your updated policy file must include the same value for `etag` as the current policy you are replacing, or the update fails. This feature prevents concurrent updates from occurring.  
   >
   > In the policy file, the value for `version` remains `1`. This version number refers to the IAM policy *schema* version, not the version of the policy. The value for `etag` is the policy version number.

3. Call [`tables.setIamPolicy`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/setIamPolicy)
   to write the updated policy.

   > [!NOTE]
   > **Note:** Empty bindings with no members are not allowed and result in an error.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the resource's [`IAM().SetPolicy()` function](https://pkg.go.dev/cloud.google.com/go/iam@v1.4.0#Handle.SetPolicy) to save changes to the access policy for a table or view.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"cloud.google.com/go/iam"
    )

    // grantAccessToResource creates a new ACL conceding the VIEWER role to the group "example-analyst-group@google.com"
    // For more information on the types of ACLs available see:
    // https://cloud.google.com/storage/docs/access-control/lists
    func grantAccessToResource(w io.Writer, projectID, datasetID, resourceID string) error {
    	// Resource can be a table or a view
    	//
    	// TODO(developer): uncomment and update the following lines:
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// resourceID := "myresource"

    	ctx := context.Background()

    	// Create new client
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	// Get resource policy.
    	policy, err := client.Dataset(datasetID).Table(resourceID).IAM().Policy(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	// Find more details about IAM Roles here:
    	// https://pkg.go.dev/cloud.google.com/go/iam#RoleName
    	entityID := "example-analyst-group@google.com"
    	roleType := iam.Viewer

    	// Add new policy.
    	policy.Add(fmt.Sprintf("group:%s", entityID), roleType)

    	// Update resource's policy.
    	err = client.Dataset(datasetID).Table(resourceID).IAM().SetPolicy(ctx, policy)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	// Get resource policy again expecting the update.
    	policy, err = client.Dataset(datasetID).Table(resourceID).IAM().Policy(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	fmt.Fprintf(w, "Details for Access entries in table or view %v.\n", resourceID)

    	for _, role := range policy.Roles() {
    		fmt.Fprintln(w)
    		fmt.Fprintf(w, "Role: %s\n", role)
    		fmt.Fprintf(w, "Entities: %v\n", policy.Members(role))
    	}

    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.Identity;
    import com.google.cloud.Policy;
    import com.google.cloud.Role;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryException;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.TableId;

    public class GrantAccessToTableOrView {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        // Project, dataset and resource (table or view) from which to get the access policy.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String resourceName = "MY_TABLE_NAME";
        // Role to add to the policy access
        Role role = Role.of("roles/bigquery.dataViewer");
        // Identity to add to the policy access
        Identity identity = Identity.user("user-add@example.com");
        grantAccessToTableOrView(projectId, datasetName, resourceName, role, identity);
      }

      public static void grantAccessToTableOrView(
          String projectId, String datasetName, String resourceName, Role role, Identity identity) {
        try {
          // Initialize client that will be used to send requests. This client only needs
          // to be created once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Create table identity given the projectId, the datasetName and the resourceName.
          TableId tableId = TableId.of(projectId, datasetName, resourceName);

          // Add new user identity to current IAM policy.
          Policy policy = bigquery.getIamPolicy(tableId);
          policy = policy.toBuilder().addIdentity(role, identity).build();

          // Update the IAM policy by setting the new one.
          bigquery.setIamPolicy(tableId, policy);

          System.out.println("IAM policy of resource \"" + resourceName + "\" updated successfully");
        } catch (BigQueryException e) {
          System.out.println("IAM policy was not updated. \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the [`Table#getIamPolicy()` function](https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table#_google_cloud_bigquery_Table_getIamPolicy_member_1_) to retrieve the current IAM policy for a table or view, modify the policy by adding new bindings, and then use [`Table#setIamPolicy()` function](https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table#setiampolicy) to save changes to the access policy.


    /**
     * TODO(developer): Update and un-comment below lines
     */
    // const projectId = "YOUR_PROJECT_ID";
    // const datasetId = "YOUR_DATASET_ID";
    // const tableId = "YOUR_TABLE_ID";
    // const principalId = "YOUR_PRINCIPAL_ID";
    // const role = "YOUR_ROLE";

    const {BigQuery} = require('@google-cloud/bigquery');

    // Instantiate a client.
    const client = new BigQuery();

    async function grantAccessToTableOrView() {
      const dataset = client.dataset(datasetId);
      const table = dataset.table(tableId);

      // Get the IAM access policy for the table or view.
      const [policy] = await table.getIamPolicy();

      // Initialize bindings array.
      if (!policy.bindings) {
        policy.bindings = [];
      }

      // To grant access to a table or view
      // add bindings to the Table or View policy.
      //
      // Find more details about Policy and Binding objects here:
      // https://cloud.google.com/security-command-center/docs/reference/rest/Shared.Types/Policy
      // https://cloud.google.com/security-command-center/docs/reference/rest/Shared.Types/Binding
      const binding = {
        role,
        members: [principalId],
      };
      policy.bindings.push(binding);

      // Set the IAM access policy with updated bindings.
      await table.setIamPolicy(policy);

      // Show a success message.
      console.log(
        `Role '${role}' granted for principal '${principalId}' on resource '${datasetId}.${tableId}'.`
      );
    }

    await grantAccessToTableOrView();

<br />

### Python


Before trying this sample, follow the Python setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquery/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the [`client.set_iam_policy()` function](https://docs.cloud.google.com/python/docs/reference/bigquery/latest/google.cloud.bigquery.client.Client#google_cloud_bigquery_client_Client_set_iam_policy) to save changes to the access policy for a table or view.

    from google.cloud import bigquery

    # TODO(developer): Update and uncomment the lines below.

    # Google Cloud Platform project.
    # project_id = "my_project_id"

    # Dataset where the table or view is.
    # dataset_id = "my_dataset"

    # Table or view name to get the access policy.
    # resource_name = "my_table"

    # Principal to grant access to a table or view.
    # For more information about principal identifiers see:
    # https://cloud.google.com/iam/docs/principal-identifiers
    # principal_id = "user:bob@example.com"

    # Role to grant to the principal.
    # For more information about BigQuery roles see:
    # https://cloud.google.com/bigquery/docs/access-control
    # role = "roles/bigquery.dataViewer"

    # Instantiate a client.
    client = bigquery.Client()

    # Get the full table or view name.
    full_resource_name = f"{project_id}.{dataset_id}.{resource_name}"

    # Get the IAM access policy for the table or view.
    policy = client.get_iam_policy(full_resource_name)

    # To grant access to a table or view, add bindings to the IAM policy.
    #
    # Find more details about Policy and Binding objects here:
    # https://cloud.google.com/security-command-center/docs/reference/rest/Shared.Types/Policy
    # https://cloud.google.com/security-command-center/docs/reference/rest/Shared.Types/Binding
    binding = {
        "role": role,
        "members": [principal_id, ],
    }
    policy.bindings.append(binding)

    # Set the IAM access policy with updated bindings.
    updated_policy = client.set_iam_policy(full_resource_name, policy)

    # Show a success message.
    print(
        f"Role '{role}' granted for principal '{principal_id}'"
        f" on resource '{full_resource_name}'."
    )

<br />

### Predefined roles that grant access to tables and views

Views are treated as table resources in BigQuery. For
fine-grained access control, you can grant a predefined or custom
IAM role on a specific table or view. The table or view also
inherits access controls specified at the dataset level and higher. For example,
if you grant a principal the BigQuery Data Owner role on a dataset, that
principal also has Data Owner permissions on the tables and views in the
dataset.

The following predefined IAM roles have permissions on tables or
views.

> [!CAUTION]
> **Caution:** While it is possible to grant BigQuery Admin or BigQuery Studio Admin permissions to a table or view, you shouldn't grant these roles at the table or view level. BigQuery Data Owner also grants all permissions for tables and views and is a less permissive role. BigQuery Admin and BigQuery Studio Admin are typically granted at the project level.

| Role | Description |
|---|---|
| [BigQuery Data Owner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner) (`roles/bigquery.dataOwner`) | When granted on a table or view, this role grants these permissions: - All permissions for the table or view. - All permissions for row access policies except permission to override time travel restrictions. - Set categories and column-level data policies. > [!NOTE] > **Note:** Principals that are granted the Data Owner role at the dataset level can also create new tables and list tables in the dataset. |
| [BigQuery Data Editor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor) (`roles/bigquery.dataEditor`) | When granted on a table or view, this role grants these permissions: - Get metadata, update metadata, get access controls, and delete the table or view. - Get (query), export, replicate, and update table data. - Create, update, and delete indexes. - Create and restore snapshots. > [!NOTE] > **Note:** Principals that are granted the Data Editor role at the dataset level can also create new tables and list tables in the dataset. |
| [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer) (`roles/bigquery.dataViewer`) | When granted on a table or view, this role grants these permissions: - Get metadata and access controls for the table or view. - Get (query), export, and replicate table data. - Create snapshots. > [!NOTE] > **Note:** Principals that are granted the Data Viewer role at the dataset level can also list tables in the dataset. |
| [BigQuery Metadata Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.metadataViewer) (`roles/bigquery.metadataViewer`) | When granted on a table or view, this role grants these permissions: - Get metadata and access controls for the table or view. > [!NOTE] > **Note:** Principals that are granted the Metadata Viewer role at the dataset level can also list tables in the dataset. |

### Permissions for tables and views

Views are treated as table resources in BigQuery. All table-level
permissions apply to views.

Most permissions that begin with `bigquery.tables` apply at the table level.
`bigquery.tables.create` and `bigquery.tables.list` don't. In order to create
and list tables or views, `bigquery.tables.create` and `bigquery.tables.list`
permissions must be granted to a role on a parent container--the dataset or the
project.

The following table lists all permissions for tables and views and the
lowest-level resource they can be granted to.

| Permission | Resource | Action |
|---|---|---|
| `bigquery.tables.create` | Dataset | Create new tables in the dataset. |
| `bigquery.tables.createIndex` | Table | Create a search index on the table. |
| `bigquery.tables.deleteIndex` | Table | Delete a search index on the table. |
| `bigquery.tables.createSnapshot` | Table | Create a snapshot of the table. Creating a snapshot requires several additional permissions at the table and dataset level. For details, see [Permissions and roles](https://docs.cloud.google.com/bigquery/docs/table-snapshots-create#permissions_and_roles) for creating table snapshots. |
| `bigquery.tables.deleteSnapshot` | Table | Delete a snapshot of the table. |
| `bigquery.tables.delete` | Table | Delete a table. |
| `bigquery.tables.createTagBinding` | Table | Create [resource tag](https://docs.cloud.google.com/bigquery/docs/tags) bindings on a table. |
| `bigquery.tables.deleteTagBinding` | Table | Delete [resource tag](https://docs.cloud.google.com/bigquery/docs/tags) bindings on a table. |
| `bigquery.tables.listTagBindings` | Table | List [resource tag](https://docs.cloud.google.com/bigquery/docs/tags) bindings on a table. |
| `bigquery.tables.listEffectiveTags` | Table | List effective tags (applied and inherited) for the table. |
| `bigquery.tables.export` | Table | Export the table's data. Running an extract job also requires `bigquery.jobs.create` permissions. |
| `bigquery.tables.get` | Table | Get metadata for a table. |
| `bigquery.tables.getData` | Table | Query the table's data. Running a query job also requires `bigquery.jobs.create` permissions. |
| `bigquery.tables.getIamPolicy` | Table | Get access controls for the table. |
| `bigquery.tables.list` | Dataset | List all tables and table metadata in the dataset. |
| `bigquery.tables.replicateData` | Table | Replicate table data. This permission is required for creating replica materialized views. |
| `bigquery.tables.restoreSnapshot` | Table | Restore a table snapshot. |
| `bigquery.tables.setCategory` | Table | Set policy tags in the table's schema. |
| `bigquery.tables.setColumnDataPolicy` | Table | Set column-level access policies on a table. |
| `bigquery.tables.setIamPolicy` | Table | Set access controls on a table. |
| `bigquery.tables.update` | Table | Update table. `metadata. bigquery.tables.get` is also required to update table metadata in the console. |
| `bigquery.tables.updateData` | Table | Update table data. |
| `bigquery.tables.updateIndex` | Table | Update a search index on the table. |

### View access controls for a table or view

To view the access controls for a table or view, choose one of the following
options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then click a table or a view.

5. Click **Share**.

   The table or view access controls appear in the **Share** pane.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To get an existing access policy and output it to a local file in JSON,
   use the
   [`bq get-iam-policy` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_get-iam-policy)
   in Cloud Shell:

   ```bash
   bq get-iam-policy \
       --table=true \
       PROJECT_ID:DATASET.RESOURCE > PATH_TO_FILE
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">DATASET</var>: the name of your dataset
   - <var translate="no">RESOURCE</var>: the name of the table or view whose policy you want to view
   - <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine

### SQL


> [!WARNING]
>
> **Preview**
>
>
> This product or feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section
> of the [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA products and features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

<br />

Query the [`INFORMATION_SCHEMA.OBJECT_PRIVILEGES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges).
Queries to retrieve access controls for a table or view must specify the
`object_schema` and `object_name`.


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   SELECT
   COLUMN_LIST
   FROM
     PROJECT_ID.`region-REGION`.INFORMATION_SCHEMA.OBJECT_PRIVILEGES
   WHERE
   object_schema = "DATASET" AND object_name = "TABLE";
   ```


   Replace the following:
   - <var translate="no">COLUMN_LIST</var>: a comma-separated list of columns from the [`INFORMATION_SCHEMA.OBJECT_PRIVILEGES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">REGION</var>: a [region qualifier](https://docs.cloud.google.com/bigquery/docs/information-schema-intro#region_qualifier)
   - <var translate="no">DATASET</var>: the name of a dataset that contains the table or view
   - <var translate="no">TABLE</var>: the name of the table or view

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

Example:

```googlesql
SELECT
object_name, privilege_type, grantee
FROM
my_project.`region-us`.INFORMATION_SCHEMA.OBJECT_PRIVILEGES
WHERE
object_schema = "mydataset" AND object_name = "mytable";
```

    +---+---+---+
    |   object_name    |  privilege_type             | grantee                  |
    +---+---+---+
    | mytable          | roles/bigquery.dataEditor   | group:group@example.com|
    | mytable          | roles/bigquery.dataOwner    | user:user@example.com|
    +---+---+---+

### API

To retrieve the current policy, call the
[`tables.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/getIamPolicy).

> [!NOTE]
> **Note:** For views, use `tables` as the value for `resource` in this API call.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the resource's [`IAM().Policy()` function](https://pkg.go.dev/cloud.google.com/go/iam@v1.4.0#Handle.Policy). Then call the [`Roles()` function](https://pkg.go.dev/cloud.google.com/go/iam@v1.4.0#Policy.Roles) to get the access policy for a table or view.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    )

    // viewTableOrViewAccessPolicies retrieves the ACL for the given resource
    // For more information on the types of ACLs available see:
    // https://cloud.google.com/storage/docs/access-control/lists
    func viewTableOrViewAccessPolicies(w io.Writer, projectID, datasetID, resourceID string) error {
    	// Resource can be a table or a view
    	//
    	// TODO(developer): uncomment and update the following lines:
    	// projectID := "my-project-id"
    	// datasetID := "my-dataset-id"
    	// resourceID := "my-resource-id"

    	ctx := context.Background()

    	// Create new client.
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	// Get resource's policy access.
    	policy, err := client.Dataset(datasetID).Table(resourceID).IAM().Policy(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	fmt.Fprintf(w, "Details for Access entries in table or view %v.\n", resourceID)

    	for _, role := range policy.Roles() {
    		fmt.Fprintln(w)
    		fmt.Fprintf(w, "Role: %s\n", role)
    		fmt.Fprintf(w, "Entities: %v\n", policy.Members(role))
    	}

    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    import com.google.cloud.Policy;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryException;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.TableId;

    public class GetTableOrViewAccessPolicy {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        // Project, dataset and resource (table or view) from which to get the access policy.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String resourceName = "MY_RESOURCE_NAME";
        getTableOrViewAccessPolicy(projectId, datasetName, resourceName);
      }

      public static void getTableOrViewAccessPolicy(
          String projectId, String datasetName, String resourceName) {
        try {
          // Initialize client that will be used to send requests. This client only needs
          // to be created once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Create table identity given the projectId, the datasetName and the resourceName.
          TableId tableId = TableId.of(projectId, datasetName, resourceName);

          // Get the table IAM policy.
          Policy policy = bigquery.getIamPolicy(tableId);

          // Show policy details.
          // Find more information about the Policy Class here:
          // https://cloud.google.com/java/docs/reference/google-cloud-core/latest/com.google.cloud.Policy
          System.out.println(
              "IAM policy info of resource \"" + resourceName + "\" retrieved succesfully");
          System.out.println();
          System.out.println("IAM policy info: " + policy.toString());
        } catch (BigQueryException e) {
          System.out.println("IAM policy info not retrieved. \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Retrieve the IAM policy for a table or view using the [`Table#getIamPolicy()` function](https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table#_google_cloud_bigquery_Table_getIamPolicy_member_1_y). The access policy details are available in the returned policy object.


    /**
     * TODO(developer): Update and un-comment below lines
     */
    // const projectId = "YOUR_PROJECT_ID"
    // const datasetId = "YOUR_DATASET_ID"
    // const resourceName = "YOUR_RESOURCE_NAME";

    const {BigQuery} = require('@google-cloud/bigquery');

    // Instantiate a client.
    const client = new BigQuery();

    async function viewTableOrViewAccessPolicy() {
      const dataset = client.dataset(datasetId);
      const table = dataset.table(resourceName);

      // Get the IAM access policy for the table or view.
      const [policy] = await table.getIamPolicy();

      // Initialize bindings if they don't exist
      if (!policy.bindings) {
        policy.bindings = [];
      }

      // Show policy details.
      // Find more details for the Policy object here:
      // https://cloud.google.com/bigquery/docs/reference/rest/v2/Policy
      console.log(`Access Policy details for table or view '${resourceName}'.`);
      console.log(`Bindings: ${JSON.stringify(policy.bindings, null, 2)}`);
      console.log(`etag: ${policy.etag}`);
      console.log(`Version: ${policy.version}`);
    }

<br />

### Revoke access to a table or view

To revoke access to a table or view, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Click **Overview \> Tables**, and then click a table or a view.

5. In the details pane, click **Share \> Manage permissions**.

6. In the **Share** dialog, expand the principal whose access you want
   to revoke.

7. Click **Delete**.

8. In the **Remove role from principal?** dialog, click **Remove**.

9. To return to the table or view details, click **Close**.

> [!NOTE]
> **Note:** If you can't revoke access, the principal may have inherited access from a higher level in the [resource hierarchy](https://docs.cloud.google.com/iam/docs/overview#resource-hierarchy).

### SQL

To remove access to tables or views from principals, use the
[`REVOKE` DCL statement](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#revoke_statement):


1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the query editor, enter the following statement:

   ```googlesql
   REVOKE `ROLE_LIST`
   ON RESOURCE_TYPE RESOURCE_NAME
   FROM "USER_LIST"
   ```


   Replace the following:
   - `ROLE_LIST`: a role or list of comma-separated roles that you want to revoke
   - `RESOURCE_TYPE`: the type of resource
     that the role is revoked from

     Supported values include
     `TABLE`, `VIEW`, `MATERIALIZED VIEW`
     and `EXTERNAL TABLE`.
   - `RESOURCE_NAME`: the name of the resource that you want to revoke permission on
   - `USER_LIST`: a comma-separated list of users
     who will have their roles revoked

     For a list of valid formats, see
     [`user_list`](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-control-language#user_list).

   <br />

3. Click **Run**.

   <br />

For more information about how to run queries, see [Run an interactive query](https://docs.cloud.google.com/bigquery/docs/running-queries#queries).

The following example revokes the BigQuery Data Owner role on `myTable`:

    REVOKE `roles/bigquery.dataOwner`
    ON TABLE `myProject`.myDataset.myTable
    FROM "group:group@example.com", "serviceAccount:user@myproject.iam.gserviceaccount.com"

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To revoke access to a table or view, use the
   [`bq remove-iam-policy-binding` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_remove-iam-policy-binding):

   ```bash
   bq remove-iam-policy-binding --member=MEMBER_TYPE:MEMBER --role=ROLE
   --table=true RESOURCE
   ```

   Replace the following:
   - <var translate="no">MEMBER_TYPE</var>: the type of member, such as `user`, `group`, `serviceAccount`, or `domain`
   - <var translate="no">MEMBER</var>: the member's email address or domain name
   - <var translate="no">ROLE</var>: the role that you want to revoke from the member
   - <var translate="no">RESOURCE</var>: the name of the table or view whose policy you want to update

### API

1. To retrieve the current policy, call the
   [`tables.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/getIamPolicy).

2. Edit the policy to remove members or bindings, or both.
   For the format required for the policy, see the [Policy](https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy)
   reference topic.

   > [!CAUTION]
   > **Caution:** When you set the policy, always retrieve the current policy as a first step to obtain the current value for `etag`. Your updated policy file must include the same value for `etag` as the current policy you are replacing, or the update fails. This feature prevents concurrent updates from occurring.  
   >
   > In the policy file, the value for `version` remains `1`. This version number refers to the IAM policy *schema* version, not the version of the policy. The value for `etag` is the policy version number.

3. Call [`tables.setIamPolicy`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/tables/setIamPolicy)
   to write the updated policy.

> [!NOTE]
> **Note:** Empty bindings with no members are not allowed and result in an error.

### Go


Before trying this sample, follow the Go setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://godoc.org/cloud.google.com/go/bigquery).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Call the [`policy.Remove()` function](https://pkg.go.dev/cloud.google.com/go/iam#Policy.Remove) to remove the access. Then call the [`IAM().SetPolicy()` function](https://pkg.go.dev/cloud.google.com/go/iam#Policy.Remove) to save changes to the access policy for a table or view.

    import (
    	"context"
    	"fmt"
    	"io"

    	"cloud.google.com/go/bigquery"
    	"cloud.google.com/go/iam"
    )

    // revokeTableOrViewAccessPolicies creates a new ACL removing the VIEWER role to group "example-analyst-group@google.com"
    // For more information on the types of ACLs available see:
    // https://cloud.google.com/storage/docs/access-control/lists
    func revokeTableOrViewAccessPolicies(w io.Writer, projectID, datasetID, resourceID string) error {
    	// Resource can be a table or a view
    	//
    	// TODO(developer): uncomment and update the following lines:
    	// projectID := "my-project-id"
    	// datasetID := "mydataset"
    	// resourceID := "myresource"

    	ctx := context.Background()

    	// Create new client
    	client, err := bigquery.NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("bigquery.NewClient: %w", err)
    	}
    	defer client.Close()

    	// Get resource policy.
    	policy, err := client.Dataset(datasetID).Table(resourceID).IAM().Policy(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	// Find more details about IAM Roles here:
    	// https://pkg.go.dev/cloud.google.com/go/iam#RoleName
    	entityID := "example-analyst-group@google.com"
    	roleType := iam.Viewer

    	// Revoke policy access.
    	policy.Remove(fmt.Sprintf("group:%s", entityID), roleType)

    	// Update resource's policy.
    	err = client.Dataset(datasetID).Table(resourceID).IAM().SetPolicy(ctx, policy)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	// Get resource policy again expecting the update.
    	policy, err = client.Dataset(datasetID).Table(resourceID).IAM().Policy(ctx)
    	if err != nil {
    		return fmt.Errorf("bigquery.Dataset.Table.IAM.Policy: %w", err)
    	}

    	fmt.Fprintf(w, "Details for Access entries in table or view %v.\n", resourceID)

    	for _, role := range policy.Roles() {
    		fmt.Fprintln(w)
    		fmt.Fprintf(w, "Role: %s\n", role)
    		fmt.Fprintf(w, "Entities: %v\n", policy.Members(role))
    	}

    	return nil
    }

<br />

### Java


Before trying this sample, follow the Java setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquery/latest/overview).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.cloud.Identity;
    import com.google.cloud.Policy;
    import com.google.cloud.Role;
    import com.google.cloud.bigquery.BigQuery;
    import com.google.cloud.bigquery.BigQueryException;
    import com.google.cloud.bigquery.BigQueryOptions;
    import com.google.cloud.bigquery.TableId;
    import java.util.HashMap;
    import java.util.HashSet;
    import java.util.Map;
    import java.util.Set;

    public class RevokeAccessToTableOrView {

      public static void main(String[] args) {
        // TODO(developer): Replace these variables before running the sample.
        // Project, dataset and resource (table or view) from which to get the access policy
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String resourceName = "MY_RESOURCE_NAME";
        // Role to remove from the access policy
        Role role = Role.of("roles/bigquery.dataViewer");
        // Identity to remove from the access policy
        Identity user = Identity.user("user-add@example.com");
        revokeAccessToTableOrView(projectId, datasetName, resourceName, role, user);
      }

      public static void revokeAccessToTableOrView(
          String projectId, String datasetName, String resourceName, Role role, Identity identity) {
        try {
          // Initialize client that will be used to send requests. This client only needs
          // to be created once, and can be reused for multiple requests.
          BigQuery bigquery = BigQueryOptions.getDefaultInstance().getService();

          // Create table identity given the projectId, the datasetName and the resourceName.
          TableId tableId = TableId.of(projectId, datasetName, resourceName);

          // Remove either identities or roles, or both from bindings and replace it in
          // the current IAM policy.
          Policy policy = bigquery.getIamPolicy(tableId);
          // Create a copy of an immutable map.
          Map<Role, Set<Identity>> bindings = new HashMap<>(policy.getBindings());

          // Remove all identities with a specific role.
          bindings.remove(role);
          // Update bindings.
          policy = policy.toBuilder().setBindings(bindings).build();

          // Remove one identity in all the existing roles.
          for (Role roleKey : bindings.keySet()) {
            if (bindings.get(roleKey).contains(identity)) {
              // Create a copy of an immutable set if the identity is present in the role.
              Set<Identity> identities = new HashSet<>(bindings.get(roleKey));
              // Remove identity.
              identities.remove(identity);
              bindings.put(roleKey, identities);
              if (bindings.get(roleKey).isEmpty()) {
                // Remove the role if it has no identities.
                bindings.remove(roleKey);
              }
            }
          }
          // Update bindings.
          policy = policy.toBuilder().setBindings(bindings).build();

          // Update the IAM policy by setting the new one.
          bigquery.setIamPolicy(tableId, policy);

          System.out.println("IAM policy of resource \"" + resourceName + "\" updated successfully");
        } catch (BigQueryException e) {
          System.out.println("IAM policy was not updated. \n" + e.toString());
        }
      }
    }

<br />

### Node.js


Before trying this sample, follow the Node.js setup instructions in the
[BigQuery quickstart using
client libraries](https://docs.cloud.google.com/bigquery/docs/quickstarts/quickstart-client-libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

Retrieve the current IAM policy for a table or view using the [`Table#getIamPolicy()`](https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table#_google_cloud_bigquery_Table_getIamPolicy_member_1_y) method. Modify the policy to remove the desired role or principal, and then apply the updated policy using the [`Table#setIamPolicy()`](https://docs.cloud.google.com/nodejs/docs/reference/bigquery/latest/bigquery/table#setiampolicy) method.


    /**
     * TODO(developer): Update and un-comment below lines
     */
    // const projectId = "YOUR_PROJECT_ID"
    // const datasetId = "YOUR_DATASET_ID"
    // const tableId = "YOUR_TABLE_ID"
    // const roleToRemove = "YOUR_ROLE"
    // const principalToRemove = "YOUR_PRINCIPAL_ID"

    const {BigQuery} = require('@google-cloud/bigquery');

    // Instantiate a client.
    const client = new BigQuery();

    async function revokeAccessToTableOrView() {
      const dataset = client.dataset(datasetId);
      const table = dataset.table(tableId);

      // Get the IAM access policy for the table or view.
      const [policy] = await table.getIamPolicy();

      // Initialize bindings array.
      if (!policy.bindings) {
        policy.bindings = [];
      }

      // To revoke access to a table or view,
      // remove bindings from the Table or View policy.
      //
      // Find more details about Policy objects here:
      // https://cloud.google.com/security-command-center/docs/reference/rest/Shared.Types/Policy

      if (principalToRemove) {
        // Create a copy of bindings for modifications.
        const bindings = [...policy.bindings];

        // Filter out the principal from each binding.
        for (const binding of bindings) {
          if (binding.members) {
            binding.members = binding.members.filter(
              m => m !== principalToRemove
            );
          }
        }

        // Filter out bindings with empty members.
        policy.bindings = bindings.filter(
          binding => binding.members && binding.members.length > 0
        );
      }

      if (roleToRemove) {
        // Filter out all bindings with the roleToRemove
        // and assign a new list back to the policy bindings.
        policy.bindings = policy.bindings.filter(b => b.role !== roleToRemove);
      }

      // Set the IAM access policy with updated bindings.
      await table.setIamPolicy(policy);

      // Both role and principal are removed
      if (roleToRemove !== null && principalToRemove !== null) {
        console.log(
          `Role '${roleToRemove}' revoked for principal '${principalToRemove}' on resource '${datasetId}.${tableId}'.`
        );
      }

      // Only role is removed
      if (roleToRemove !== null && principalToRemove === null) {
        console.log(
          `Role '${roleToRemove}' revoked for all principals on resource '${datasetId}.${tableId}'.`
        );
      }

      // Only principal is removed
      if (roleToRemove === null && principalToRemove !== null) {
        console.log(
          `Access revoked for principal '${principalToRemove}' on resource '${datasetId}.${tableId}'.`
        );
      }

      // No changes were made
      if (roleToRemove === null && principalToRemove === null) {
        console.log(
          `No changes made to access policy for '${datasetId}.${tableId}'.`
        );
      }
    }

<br />

## Work with access controls for routines

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

To provide feedback or request support for this feature, email
[bigquery-security@google.com](mailto:bigquery-security@google.com).

You can provide access to a routine by granting an [IAM
principal](https://docs.cloud.google.com/iam/docs/principal-identifiers#allow) a predefined or custom role
that determines what the principal can do with the routine. This is also known
as attaching an [allow policy](https://docs.cloud.google.com/iam/docs/allow-policies) to a resource. After
granting access, you can view the access controls for the routine, and you
can revoke access to the routine.

### Grant access to a routine

For fine-grained access control, you can grant a predefined or custom
IAM role on a specific routine. The routine also inherits access
controls specified at the dataset level and higher. For example, if you grant a
principal the BigQuery Data Owner role on a dataset, that principal also has
Data Owner permissions on the routines in the dataset.

Select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Go to the **Routines** tab and click a routine.

5. Click **Share**.

6. Click **Add members**.

7. In the **New members** field, enter a principal.

8. In the **Select a role** list, select a predefined role or a custom role.

9. Click **Save**.

10. To return to the routine info, click **Done**.

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To write the existing routine information (including access controls) to
   a JSON file, use the
   [`bq get-iam-policy` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_get-iam-policy):

   ```bash
   bq get-iam-policy \
       PROJECT_ID:DATASET.ROUTINE \
       > PATH_TO_FILE
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">DATASET</var>: the name of the dataset that contains the routine that you want to update
   - <var translate="no">ROUTINE</var>: the name of the resource to update
   - <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine

   > [!CAUTION]
   > **Caution:** When you set the policy, always retrieve the current policy as a first step to obtain the current value for `etag`. Your updated policy file must include the same value for `etag` as the current policy you are replacing, or the update fails. This feature prevents concurrent updates from occurring.  
   >
   > In the policy file, the value for `version` remains `1`. This version number refers to the IAM policy *schema* version, not the version of the policy. The value for `etag` is the policy version number.

3. Make changes to the `bindings` section of the JSON file. A binding binds
   one or more principals to a single `role`. Principals can
   be user accounts, service accounts, Google groups, and domains. For
   example, the `bindings` section of a routine's JSON file would look like
   the following:

   ```json
   {
     "bindings": [
       {
         "role": "roles/bigquery.dataViewer",
         "members": [
           "user:user@example.com",
           "group:group@example.com",
           "domain:example.com",
         ]
       },
     ],
     "etag": "BwWWja0YfJA=",
     "version": 1
   }
   ```

   > [!NOTE]
   > **Note:** Empty bindings with no members are not allowed and result in an error.

4. To update the access policy, use the `bq set-iam-policy` command:

   ```bash
   bq set-iam-policy PROJECT_ID:DATASET.ROUTINE PATH_TO_FILE
   ```
5. To verify your access control changes, use the
   [`bq get-iam-policy` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_get-iam-policy)
   again without writing the information to a file:

   ```bash
   bq get-iam-policy --format=prettyjson \\
       PROJECT_ID:DATASET.ROUTINE
   ```

### API

1. To retrieve the current policy, call the
   [`routines.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy).

2. Edit the policy to add principals, bindings, or both.
   For the format required for the policy, see the [Policy](https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy)
   reference topic.

   > [!CAUTION]
   > **Caution:** Before you set the policy, always retrieve the current policy to obtain the current value for `etag`. Your updated policy file must include the same value for `etag` as the current policy you are replacing, or the update fails. This feature prevents concurrent updates from occurring.  
   >
   > In the policy file, the value for `version` remains `1`. This version number refers to the IAM policy *schema* version, not the version of the policy. The value for `etag` is the policy version number.

3. Call
   [`routines.setIamPolicy`](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/setIamPolicy)
   to write the updated policy.

> [!NOTE]
> **Note:** Empty bindings with no members are not allowed and result in an error.

### Predefined roles that grant access to routines

For fine-grained access control, you can grant a predefined or custom
IAM role on a specific routine. The routine also inherits access
controls specified at the dataset level and higher. For example, if you grant a
principal the Data Owner role on a dataset, that principal also has Data Owner
permissions on the routines in the dataset through inheritance.

The following predefined IAM roles have permissions on routines.

> [!CAUTION]
> **Caution:** While it is possible to grant BigQuery Admin or BigQuery Studio Admin permissions on a routine, you shouldn't grant these roles at the routine level. BigQuery Data Editor also grants all permissions for the routine and is a less permissive role. BigQuery Admin and BigQuery Studio Admin are typically granted at the project level.

| Role | Description |
|---|---|
| [BigQuery Data Owner](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataOwner) (`roles/bigquery.dataOwner`) | When granted on a routine, this role grants these permissions: - All permissions for the routine. You shouldn't grant the Data Owner role at the routine level. Data Editor also grants all permissions for the routine and is a less permissive role. > [!NOTE] > **Note:** Principals that are granted the Data Owner role at the dataset level can create routines and list routines in the dataset. |
| [BigQuery Data Editor](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataEditor) (`roles/bigquery.dataEditor`) | When granted on a routine, this role grants these permissions: - All permissions for the routine. > [!NOTE] > **Note:** Principals that are granted the Data Editor role at the dataset level can create routines and list routines in the dataset. |
| [BigQuery Data Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.dataViewer) (`roles/bigquery.dataViewer`) | When granted on a routine, this role grants these permissions: - In a query, reference a routine created by someone else. > [!NOTE] > **Note:** Principals that are granted the Data Viewer role at the dataset level can also list routines in the dataset. |
| [BigQuery Metadata Viewer](https://docs.cloud.google.com/bigquery/docs/access-control#bigquery.metadataViewer) (`roles/bigquery.metadataViewer`) | When granted on a routine, this role grants these permissions: - In a query, reference a routine created by someone else. > [!NOTE] > **Note:** Principals that are granted the Metadata Viewer role at the dataset level can also list routines in the dataset. |

### Permissions for routines

Most permissions that begin with `bigquery.routines` apply at the routine level.
`bigquery.routines.create` and `bigquery.routines.list` don't. In order to
create and list routines, `bigquery.routines.create` and
`bigquery.routines.list` permissions must be granted to a role on the parent
container--the dataset.

The following table lists all permissions for routines and the lowest-level
resource they can be granted to.

| Permission | Resource | Description |
|---|---|---|
| `bigquery.routines.create` | Dataset | Create a routine in the dataset. This permission also requires `bigquery.jobs.create` to run a query job that contains a `CREATE FUNCTION` statement. |
| `bigquery.routines.delete` | Routine | Delete a routine. |
| `bigquery.routines.get` | Routine | Reference a routine created by someone else. This permission also requires `bigquery.jobs.create` to run a query job that references the routine, and you also need permission to access any resources that the routine references, such as tables or views. |
| `bigquery.routines.list` | Dataset | List routines in the dataset and show metadata for routines. |
| `bigquery.routines.update` | Routine | Update routine definitions and metadata. |
| `bigquery.routines.getIamPolicy` | Routine | Get access controls for the routine. |
| `bigquery.routines.setIamPolicy` | Routine | Set access controls for the routine. |

### View the access controls for a routine

To view the access controls for a routine, choose one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Go to the **Routines** tab and click a routine.

5. Click **Share**.

   The routine's access controls appear in the **Share** pane.

### bq

The `bq get-iam-policy` command does not provide support for viewing access
controls on a routine.

### SQL

The
[`INFORMATION_SCHEMA.OBJECT_PRIVILEGES` view](https://docs.cloud.google.com/bigquery/docs/information-schema-object-privileges)
doesn't show access controls for routines.

### API

To retrieve the current policy, call the
[`routines.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy).

### Revoke access to a routine

To revoke access to a routine, select one of the following options:

### Console

1. Go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset.

4. Go to the **Routines** tab and click a routine.

5. In the details pane, click **Share \> Permissions**.

6. In the **Routine Permissions** dialog, expand the principal whose access
   you want to revoke.

7. Click **Remove principal**.

8. In the **Remove role from principal?** dialog, click **Remove**.

9. Click **Close**.

> [!NOTE]
> **Note:** If you are unable to revoke access, the principal may have inherited access from a higher level in the [resource hierarchy](https://docs.cloud.google.com/iam/docs/overview#resource-hierarchy). For instructions on editing principal access, see [Grant or revoke a single role](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access#single-role).

### bq


1. In the Google Cloud console, activate Cloud Shell.

   [Activate Cloud Shell](https://console.cloud.google.com/?cloudshell=true)


   At the bottom of the Google Cloud console, a
   [Cloud Shell](https://docs.cloud.google.com/shell/docs/how-cloud-shell-works)
   session starts and displays a command-line prompt. Cloud Shell is a shell environment
   with the Google Cloud CLI
   already installed and with values already set for
   your current project. It can take a few seconds for the session to initialize.
2. To write the existing routine information (including access controls) to
   a JSON file, use the
   [`bq get-iam-policy` command](https://docs.cloud.google.com/bigquery/docs/reference/bq-cli-reference#bq_get-iam-policy):

   ```bash
   bq get-iam-policy --routine PROJECT_ID:DATASET.ROUTINE > PATH_TO_FILE
   ```

   Replace the following:
   - <var translate="no">PROJECT_ID</var>: your project ID
   - <var translate="no">DATASET</var>: the name of the dataset that contains the routine that you want to update
   - <var translate="no">ROUTINE</var>: the name of the resource to update
   - <var translate="no">PATH_TO_FILE</var>: the path to the JSON file on your local machine

   > [!CAUTION]
   > **Caution:** When you set the policy, always retrieve the current policy as a first step to obtain the current value for `etag`. Your updated policy file must include the same `etag` value as the current policy that you are replacing, or the update fails. This feature prevents concurrent updates from occurring.

   In the policy file, the value for `version` remains `1`. This number
   refers to the IAM policy *schema* version, not the
   version of the policy. The value for `etag` value is the policy
   version number.
3. Make changes to the `access` section of the JSON file. You can remove
   any of the `specialGroup` entries: `projectOwners`, `projectWriters`,
   `projectReaders`, and `allAuthenticatedUsers`. You can also remove any
   of the following: `userByEmail`, `groupByEmail`, and `domain`.

   For example, the `access` section of a routine's JSON file would look
   like the following:

   ```json
   {
    "bindings": [
      {
        "role": "roles/bigquery.dataViewer",
        "members": [
          "user:user@example.com",
          "group:group@example.com",
          "domain:google.com",
        ]
      },
    ],
    "etag": "BwWWja0YfJA=",
    "version": 1
   }
   ```

   > [!NOTE]
   > **Note:** Empty bindings with no members are not allowed and result in an error.

4. To update the access policy, use the `bq set-iam-policy` command:

   ```bash
   bq set-iam-policy --routine PROJECT_ID:DATASET.ROUTINE PATH_TO_FILE
   ```
5. To verify your access control changes, use the `get-iam-policy` command
   again without writing the information to a file:

   ```bash
   bq get-iam-policy --routine --format=prettyjson PROJECT_ID:DATASET.ROUTINE
   ```

### API

1. To retrieve the current policy, call the
   [`routines.getIamPolicy` method](https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/routines/getIamPolicy).

2. Edit the policy to add principals or bindings, or both.
   For the format required for the policy, see the [Policy](https://docs.cloud.google.com/iam/docs/reference/rest/v1/Policy)
   reference topic.

   > [!CAUTION]
   > **Caution:** When you set the policy, always retrieve the current policy as a first step to obtain the current value for `etag`. Your updated policy file must include the same value for `etag` as the current policy you are replacing, or the update fails. This feature prevents concurrent updates from occurring.  
   >
   > In the policy file, the value for `version` remains `1`. This version number refers to the IAM policy *schema* version, not the version of the policy. The value for `etag` is the policy version number.

## View inherited access controls for a resource

You can examine the inherited IAM roles for a resource by using
the BigQuery web UI. You'll need the [appropriate permissions to view inheritance](https://docs.cloud.google.com/iam/docs/resource-hierarchy-access-control#view-inherited-policies)
in the console. To examine inheritance for a dataset, table, view, or routine:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project, click **Datasets**, and
   then select a dataset, or select a table, view, or routine in the dataset.

4. Click **Share \> Manage permissions**.

5. Verify that the **Show inherited roles in table** option is enabled.

   ![The Show inherited roles in table option in the console](https://docs.cloud.google.com/bigquery/images/inheritance-toggle.png)
6. Expand a role in the table.

7. In the **Inheritance** column, the hexagonal icon indicates whether the role
   was inherited from a parent resource.

   ![The Inherited from a parent resource icon](https://docs.cloud.google.com/bigquery/images/inherited-icon.png)

> [!NOTE]
> **Note:** You can also [view all allow and deny policies](https://docs.cloud.google.com/iam/docs/troubleshoot-policies#applicable-policies) for a resource's parent project, folder, and organization by using the [`gcloud beta projects get-ancestors-iam-policy` command](https://docs.cloud.google.com/sdk/gcloud/reference/beta/projects/get-ancestors-iam-policy).

## Deny access to a resource

> [!WARNING]
>
> **Preview**
>
>
> This feature is
>
> subject to the "Pre-GA Offerings Terms" in the General Service Terms section of the
> [Service Specific
> Terms](https://docs.cloud.google.com/terms/service-terms#1).
>
> Pre-GA features are available "as is" and might have limited support.
>
> For more information, see the
> [launch stage descriptions](https://cloud.google.com/products/#product-launch-stages).

> [!NOTE]
> **Note:** To provide feedback or ask questions that are related to this preview release, contact [bigquery-security@google.com](mailto:bigquery-security@google.com).

[IAM deny policies](https://docs.cloud.google.com/iam/docs/deny-overview) let you set
guardrails on access to BigQuery resources. You can define deny rules
that prevent selected principals from using
[certain permissions](https://docs.cloud.google.com/iam/docs/deny-permissions-support), regardless of
the roles they're granted.

For information about how to create, update, and delete deny policies, see
[Deny access to resources](https://docs.cloud.google.com/iam/docs/deny-access).

### Special cases

Consider the following scenarios when you create [IAM deny policies](https://docs.cloud.google.com/iam/docs/deny-overview)
on a few BigQuery permissions:

- Access to authorized resources ([views](https://docs.cloud.google.com/bigquery/docs/authorized-views),
  [routines](https://docs.cloud.google.com/bigquery/docs/authorized-routines), [datasets](https://docs.cloud.google.com/bigquery/docs/authorized-datasets),
  or [stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures#authorize_routines)) lets you
  [create](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#create_table_statement),
  [drop](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/data-definition-language#drop_table_statement),
  or [manipulate](https://docs.cloud.google.com/bigquery/docs/reference/standard-sql/dml-syntax) a table,
  along with reading and modifying table data, even if you don't have direct
  permission to perform those operations. It can also
  [get model data or metadata](https://docs.cloud.google.com/bigquery/docs/getting-model-metadata) and
  [invoke other stored procedures](https://docs.cloud.google.com/bigquery/docs/procedures#call_a_stored_procedure)
  on the underlying table. This capability implies that the authorized resources
  have the following permissions:

  - `bigquery.tables.get`
  - `bigquery.tables.list`
  - `bigquery.tables.getData`
  - `bigquery.tables.updateData`
  - `bigquery.tables.create`
  - `bigquery.tables.delete`
  - `bigquery.routines.get`
  - `bigquery.routines.list`
  - `bigquery.datasets.get`
  - `bigquery.models.getData`
  - `bigquery.models.getMetadata`

  To deny access to these authorized resources, add one of the
  following values to the [`deniedPrincipal`](https://docs.cloud.google.com/iam/docs/deny-overview#deny-rules)
  field when you create the deny policy:

  | Value | Use case |
  |---|---|
  | `principalSet://goog/public:all` | Blocks all principals including authorized resources. |
  | `principalSet://bigquery.googleapis.com/projects/PROJECT_NUMBER/*` | Blocks all BigQuery authorized resources in the specified project. [`PROJECT_NUMBER`](https://docs.cloud.google.com/resource-manager/docs/view-update-projects#identifying_projects) is an automatically generated unique identifier for your project of type `INT64`. |

- To exempt certain principals from the deny policy, specify those
  principals in the [`exceptionPrincipals`](https://docs.cloud.google.com/iam/docs/deny-overview#deny-rules)
  field of your deny policy. For example, `exceptionPrincipals: "principalSet://bigquery.googleapis.com/projects/1234/*"`.

- BigQuery [caches query results](https://docs.cloud.google.com/bigquery/docs/cached-results#how_cached_results_are_stored)
  of a job owner for 24 hours, which the job owner can access without needing
  the `bigquery.tables.getData` permission on the table containing the
  data. Hence, adding an IAM deny policy to the
  `bigquery.tables.getData` permission doesn't block access to cached results
  for the job owner until the cache expires. To block the job owner access to
  cached results, create a separate deny policy on the `bigquery.jobs.create`
  permission.

- To prevent unintended data access when using deny policies to block data read
  operations, we recommend that you also review and revoke any existing
  subscriptions on the dataset.

- To create a [IAM deny policy](https://docs.cloud.google.com/iam/docs/deny-overview) for
  viewing dataset access controls, deny the following permissions:

  - `bigquery.datasets.get`
  - `bigquery.datasets.getIamPolicy`
- To create a [IAM deny policy](https://docs.cloud.google.com/iam/docs/deny-overview) for
  updating dataset access controls, deny the following permissions:

  - `bigquery.datasets.update`
  - `bigquery.datasets.setIamPolicy`

## What's next

Learn how to use the
[`projects.testIamPermissions` method](https://docs.cloud.google.com/resource-manager/reference/rest/v1/projects/testIamPermissions)
to test user access to a resource.