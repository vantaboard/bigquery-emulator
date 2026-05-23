# Create and manage repositories

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

> [!NOTE]
> **Note:** To provide feedback or ask questions that are related to this Preview feature, contact [bigquery-repositories-feedback@google.com](mailto:bigquery-repositories-feedback@google.com).

This document shows you how to work with repositories in BigQuery,
including the following tasks:

- Creating repositories
- Deleting repositories
- Sharing repositories
- Optionally connecting a BigQuery repository to a third-party repository

## Before you begin

<br />

### Required roles


To get the permissions that
you need to work with a repositories and workspaces,

ask your administrator to grant you the
following IAM roles on repositories and workspaces:

- Create and manage shared repositories: [Code Owner](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeOwner) (`roles/dataform.codeOwner`)
- Create and delete workspaces in shared repositories: [Code Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeEditor) (`roles/dataform.codeEditor`)
- Create, modify, and version control files in workspaces in shared repositories: [Code Editor](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeEditor) (`roles/dataform.codeEditor`)
- View workspaces and their files in shared repositories: [Code Viewer](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeViewer) (`roles/dataform.codeViewer`)
- Create and manage private repositories, including all actions with workspaces and files in the private repository: [Code Creator](https://docs.cloud.google.com/iam/docs/roles-permissions/dataform#dataform.codeCreator) (`roles/dataform.codeCreator`)


For more information about granting roles, see [Manage access to projects, folders, and organizations](https://docs.cloud.google.com/iam/docs/granting-changing-revoking-access).


You might also be able to get
the required permissions through [custom
roles](https://docs.cloud.google.com/iam/docs/creating-custom-roles) or other [predefined
roles](https://docs.cloud.google.com/iam/docs/roles-overview#predefined).

Principals that have the Code Editor role on a repository are able to
edit all workspaces in the repository.

Private repositories that you create are still visible to principals who are
granted the BigQuery Admin or BigQuery Studio Admin roles at the project level.
These principals can share your private repository with other users.

## Create a repository

To create a BigQuery repository, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)

   If you don't see the left pane, click **Expand left pane** to open the pane.
3. In the **Explorer** pane, expand your project and then click
   **Repositories** to open the **Repositories** tab in the details pane.

4. Click **Add Repository**.

5. In the **Create repository** pane, in the **Repository ID** field,
   type a unique ID.

   IDs can only include numbers, letters, hyphens, and underscores.
6. In the **Region** drop-down list, select a BigQuery region
   for storing the repository and its contents. Select the BigQuery
   region nearest to your location.

   For a list of available BigQuery regions, see
   [BigQuery Studio locations](https://docs.cloud.google.com/bigquery/docs/locations#bqstudio-loc). The
   repository region does not have to match the location of your
   BigQuery datasets.
7. Click **Create**.

## Connect to a third-party repository

This section shows you how to connect a BigQuery repository to a
remote repository. After you connect the repositories, you can perform Git
actions on the files in the workspaces contained by the repository.
For example, pulling updates from the remote repository and pushing changes
to the remote repository.

We recommend creating a dedicated BigQuery repository for each
remote repository that you connect to. Give the BigQuery
repository a similar name to the remote repository to help make the mapping
clear.

You can connect a remote repository through HTTPS or SSH. Connecting a
BigQuery repository to a remote repository
can fail if the remote repository is not open to the public internet,
for example, if it is behind a firewall. The following table
lists supported Git providers and the connection methods that are available
for their repositories:

| Git provider | Connection method |
|---|---|
| Azure DevOps Services | SSH |
| Bitbucket | SSH |
| GitHub | SSH or HTTPS |
| GitLab | SSH or HTTPS |

> [!IMPORTANT]
> **Important:** To create a BigQuery repository connected to a remote Git repository that is not allow-listed in the `dataform.restrictGitRemotes` policy, first add the remote Git repository to the `allowedValues` list in the policy, and then create a new BigQuery repository and connect it to the remote repository. For more information, see [Restrict remote repositories](https://docs.cloud.google.com/dataform/docs/restrict-git-remotes).

### Connect a remote repository through SSH

To connect a remote repository through SSH, you must generate an SSH key and a
Secret Manager secret. The SSH key consists of a public SSH key and a
private SSH key. You must share the public SSH key with your Git provider,
and create a Secret Manager secret with the private SSH key. Then,
share the secret with your custom service account.

BigQuery uses the secret with the private SSH key to sign in to
your Git provider to commit changes on behalf of users. BigQuery
makes these commits using the user's Google Cloud email address so you can tell
who made each commit.

> [!WARNING]
> **Warning:** The private SSH key is shared among all BigQuery users who have [act-as permissions](https://docs.cloud.google.com/dataform/docs/strict-act-as-mode) on the custom service account. We recommend that you create a machine user with your Git provider and limit its access to the remote Git repositories that you plan to use with BigQuery. Only Google Cloud project owners and BigQuery users with the [Code Owner role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeOwner) (`roles/dataform.codeOwner`) can use the SSH key to connect repositories. BigQuery users aren't able to see the SSH key itself.

To connect a remote repository to a
BigQuery repository through SSH, follow these steps:

1. In your Git provider, do the following:

   ### Azure DevOps Services

   1. In Azure DevOps Services, [create a private SSH key](https://learn.microsoft.com/en-us/azure/devops/repos/git/use-ssh-keys-to-authenticate?view=azure-devops#step-1-create-your-ssh-keys).
   2. [Upload the public SSH key](https://learn.microsoft.com/en-us/azure/devops/repos/git/use-ssh-keys-to-authenticate?view=azure-devops#step-2-add-the-public-key-to-azure-devops) to your Azure DevOps Services repository.

   ### Bitbucket

   1. In Bitbucket, [create a private SSH key](https://support.atlassian.com/bitbucket-cloud/docs/configure-ssh-and-two-step-verification/).
   2. [Upload the public SSH key](https://support.atlassian.com/bitbucket-cloud/docs/configure-ssh-and-two-step-verification/) to your Bitbucket repository.

   ### GitHub

   1. In GitHub, [check for existing SSH keys](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/checking-for-existing-ssh-keys).
   2. If you don't have any existing SSH keys, or you'd like to use a new key, [create a private SSH key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).
   3. [Upload the GitHub public SSH key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account) to your GitHub repository.

   ### GitLab

   1. In GitLab, [create a private SSH key](https://docs.gitlab.com/ee/user/ssh.html#generate-an-ssh-key-pair).
   2. [Upload the GitLab public SSH key](https://docs.gitlab.com/ee/user/ssh.html#add-an-ssh-key-to-your-gitlab-account) to your GitLab repository.
2. In Secret Manager, [create a secret](https://docs.cloud.google.com/secret-manager/docs/creating-and-accessing-secrets#create)
   and paste in your private SSH key as the secret value. Your private SSH
   key should be stored in a file similar to `~/.ssh/id_ed25519`. Give a name
   to the secret so you can find it in the future.

3. [Grant access to the secret to your default Dataform service agent](https://docs.cloud.google.com/secret-manager/docs/manage-access-to-secrets).

   Your default Dataform service agent is in the following format:

       service-PROJECT_NUMBER@gcp-sa-dataform.iam.gserviceaccount.com

4. Grant the
   [`roles/secretmanager.secretAccessor` role](https://docs.cloud.google.com/secret-manager/docs/access-control#secretmanager.secretAccessor)
   to the service account.

5. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
6. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
7. In the **Explorer** pane, expand your project and then click
   **Repositories** to open the **Repositories** tab in the details pane.

8. Select the BigQuery repository that you want to connect
   to the remote repository.

9. In the editor, select the **Configuration** tab.

10. Click **Connect with Git**.

11. In the **Connect to remote repository** pane, select the **SSH** radio
    button.

12. In the **Remote Git repository URL** field, type the URL of the remote Git
    repository, ending with `.git`.

    The URL of the remote Git repository must be in one of
    the following formats:
    - Absolute URL: `ssh://git@{host_name}[:{port}]/{repository_path}`, `port` is optional.
    - SCP-like URL: `git@{host_name}:{repository_path}`.
13. In the **Default remote branch name** field, type the name
    of the main branch of the remote Git repository.

14. In the **Secret** drop-down, select the secret that you created that contains
    the SSH private key.

15. In the **SSH public host key value** field, type the public host key of your
    Git provider.

    ### Azure DevOps Services

    1. To retrieve the Azure DevOps Services public host key, run the following command in the terminal:

           ssh-keyscan -t rsa ssh.dev.azure.com

    2. Copy one of the outputted keys, omitting `ssh.dev.azure.com` from the beginning of the line.
       The value that you copy must be in the following format:

           ALGORITHM BASE64_KEY_VALUE

       For example:

           ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC7Hr1oTWqNqOlzGJOfGJ4NakVyIzf1rXYd4d7wo6jBlkLvCA4odBlL0mDUyZ0/QUfTTqeu+tm22gOsv+VrVTMk6vwRU75gY/y9ut5Mb3bR5BV58dKXyq9A9UeB5Cakehn5Zgm6x1mKoVyf+FFn26iYqXJRgzIZZcZ5V6hrE0Qg39kZm4az48o0AUbf6Sp4SLdvnuMa2sVNwHBboS7EJkm57XQPVU3/QpyNLHbWDdzwtrlS+ez30S3AdYhLKEOxAG8weOnyrtLJAUen9mTkol8oII1edf7mWWbWVf0nBmly21+nZcmCTISQBtdcyPaEno7fFQMDD26/s0lfKob4Kw8H

       Verify this key is still up-to-date with Azure DevOps Services.

    ### Bitbucket

    1. To retrieve the Bitbucket public host key, run the following command in the terminal:

           curl https://bitbucket.org/site/ssh

    2. The command returns a list of public host keys. Choose one of the keys from the list, and copy it, omitting `bitbucket.org` from the beginning of the line.
       The value that you copy must be in the following format:

           ALGORITHM BASE64_KEY_VALUE

       For example:

           ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIIazEu89wgQZ4bqs3d63QSMzYVa0MuJ2e2gKTKqu+UUO

       Verify this key is still up-to-date with Bitbucket.

    ### GitHub

    1. To retrieve the GitHub public host key, see [GitHub's SSH key fingerprints](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/githubs-ssh-key-fingerprints).
    2. The page contains a list of public host keys. Choose one of them, and copy it, omitting `github.com` from the beginning of the line.
       The value that you copy must be in the following format:

           ALGORITHM BASE64_KEY_VALUE

       For example:

           ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOMqqnkVzrm0SdG6UOoqKLsabgH5C9okWi0dh2l9GKJl

       Verify this key is still up-to-date with GitHub.

    ### GitLab

    1. To retrieve the GitLab public host key, see [SSH `known_hosts` entries](https://docs.gitlab.com/ee/user/gitlab_com/#ssh-known_hosts-entries).
    2. The page contains a list of public host keys. Choose one of them, and copy it, omitting `gitlab.com` from the beginning of the line.
       The value that you copy must be in the following format:

           ALGORITHM BASE64_KEY_VALUE

       For example:

           ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIAfuCHKVTjquxvt6CM6tdG4SLp1Btn/nOeHHE5UOzRdf

       Verify this key is still up-to-date with GitLab.
16. Click **Connect**.

### Connect a remote repository through HTTPS

To connect a remote repository through HTTPS, you must create a
Secret Manager secret with a personal access token, and share the
secret with your custom service account.

BigQuery then uses the access token to sign in to your Git
provider to commit changes on behalf of users. BigQuery makes
these commits using the user's Google Cloud email address so you can tell
who made each commit.

> [!WARNING]
> **Warning:** The private HTTPS token is shared among all BigQuery users who have [act-as permissions](https://docs.cloud.google.com/dataform/docs/strict-act-as-mode) on the custom service account. We recommend that you create a machine user with your Git provider and limit its access to the remote Git repositories that you plan to use with BigQuery. Only Google Cloud project owners and BigQuery users with the [Code Owner role](https://docs.cloud.google.com/dataform/docs/access-control#dataform.codeOwner) (`roles/dataform.codeOwner`) can use the HTTPS token to connect repositories. BigQuery users aren't able to see the HTTPS token itself.

To connect a remote repository to a BigQuery repository through
HTTPS, follow these steps:

1. In your Git provider, do the following:

   ### GitHub

   1. In GitHub, create a [fine-grained personal access token](https://github.blog/2022-10-18-introducing-fine-grained-personal-access-tokens-for-github/)
      or a [classic personal access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token#about-personal-access-tokens).

      - For a fine-grained GitHub personal access token, do the following:

      1. Select repository access to only selected repositories, then select the
         repository that you want to connect to.

      2. Grant read and write access on contents of the repository.

      3. Set a token expiration time appropriate to your needs.

      - For a classic GitHub personal access token, do the following:

      1. Grant BigQuery the `repo` permission.

      2. Set a token expiration time appropriate to your needs.

   2. If your organization uses SAML single sign-on (SSO),
      [authorize the token](https://docs.github.com/en/enterprise-cloud@latest/authentication/authenticating-with-saml-single-sign-on/authorizing-a-personal-access-token-for-use-with-saml-single-sign-on).

   ### GitLab

   1. In GitLab, create a [GitLab personal access token](https://docs.gitlab.com/ee/user/profile/personal_access_tokens.html).

   2. Name the token `dataform`; this is required.

   3. Grant BigQuery the `api`, `read_repository`,
      and `write_repository` permissions.

   4. Set a token expiration time appropriate to your needs.

2. In Secret Manager,
   [create a secret](https://docs.cloud.google.com/secret-manager/docs/creating-and-accessing-secrets#create)
   containing the personal access token of your remote repository.

3. [Grant access to the secret to your default Dataform service agent](https://docs.cloud.google.com/secret-manager/docs/manage-access-to-secrets).

   Your default Dataform service agent is in the following format:

       service-PROJECT_NUMBER@gcp-sa-dataform.iam.gserviceaccount.com

4. Grant the
   [`roles/secretmanager.secretAccessor` role](https://docs.cloud.google.com/secret-manager/docs/access-control#secretmanager.secretAccessor)
   to the service account.

5. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
6. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
7. In the **Explorer** pane, expand your project and then click
   **Repositories** to open the **Repositories** tab in the details pane.

8. Select the BigQuery repository that you want to connect
   to the remote repository.

9. In the editor, select the **Configuration** tab.

10. Click **Connect with Git**.

11. In the **Connect to remote repository** pane, select the **HTTPS** radio
    button.

12. In the **Remote Git repository URL** field, type the URL of the remote Git
    repository, ending with `.git`.

    The URL of the remote Git repository can't contain usernames or passwords.
13. In the **Default remote branch name** field, type the name
    of the main branch of the remote Git repository.

14. In the **Secret** drop-down, select the secret that you created that contains
    the personal access token.

15. Click **Connect**.

### Edit the remote repository connection

To edit a connection between a BigQuery repository and a remote
Git repository, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and then click
   **Repositories** to open the **Repositories** tab in the details pane.

4. Select the BigQuery repository whose connection you want to
   edit.

5. In the editor, select the **Configuration** tab.

6. On the repository page, click **Edit Git connection**.

7. Edit connection settings.

8. Click **Update**.

## Share a repository

To share a repository, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and then click
   **Repositories** to open the **Repositories** tab in the details pane.

4. Find the repository that you want to share.

5. Click
   **Open actions** and then click **Share**.

6. In the **Share permissions** pane, click **Add User/Group**.

7. In the **Add User/Group** pane, in the **New Principals** field, type one
   or more user or group names, separated by commas.

8. In the **Role** field, choose the role to assign to the new principals.

9. Click **Save**.

> [!NOTE]
> **Note:** If you enhance security by setting the `enable_private_workspace` field [(Preview)](https://cloud.google.com/products#product-launch-stages) to `true` in the [`projects.locations.updateConfig` Dataform API method](https://docs.cloud.google.com/dataform/reference/rest/v1beta1/projects.locations/updateConfig) for your Google Cloud project, then for any Dataform repository used by your BigQuery repository, only the creator of a Dataform workspace in that Dataform repository can read and write code in that Dataform workspace.

## Delete a repository

To delete a repository and all its contents, follow these steps:

1. In the Google Cloud console, go to the **BigQuery** page.

   [Go to BigQuery](https://console.cloud.google.com/bigquery)
2. In the left pane, click **Explorer**:

   ![Highlighted button for the Explorer pane.](https://docs.cloud.google.com/static/bigquery/images/explorer-tab.png)
3. In the **Explorer** pane, expand your project and then click
   **Repositories** to open the **Repositories** tab in the details pane.

4. Find the repository that you want to delete.

5. Click
   **Open actions** and then click **Delete**.

6. Click **Delete**.

## What's next

- Learn how to [create workspaces](https://docs.cloud.google.com/bigquery/docs/workspaces).