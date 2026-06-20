import { expect, test, type Page } from '@playwright/test';

async function expandProject(page: Page) {
    const project = page.getByTestId('project-local-project');
    await expect(project).toBeVisible();
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/bigquery/v2/projects/local-project/datasets') && r.ok()),
        project.click(),
    ]);
}

async function expandDataset(page: Page) {
    await expandProject(page);
    const toggle = page.getByTestId('dataset-toggle-test-dataset');
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset/tables') && r.ok()),
        toggle.click(),
    ]);
}

async function openDataset(page: Page) {
    await expandProject(page);
    const dataset = page.getByTestId('dataset-test-dataset');
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset') && r.ok()),
        dataset.click(),
    ]);
    await expect(page.getByTestId('dataset-tab-page')).toBeVisible();
    await expect(page.getByTestId('dataset-tab-overview')).toBeVisible();
}

async function selectTable(page: Page) {
    await expandDataset(page);
    const table = page.getByTestId('table-table_a');
    await table.click();
    await expect(page.getByTestId('table-tab-page')).toBeVisible();
}

async function openQueryFromTable(page: Page) {
    await selectTable(page);
    await expect(page.getByTestId('table-tab-page')).toBeVisible();
    await page.getByTestId('open-query-from-table').click();
    await expect(page.getByTestId('sql-editor')).toBeVisible();
}

async function openTableSchemaTab(page: Page) {
    await selectTable(page);
    await expect(page.getByTestId('table-tab-schema')).toBeVisible();
}

test.describe('BigQuery Explorer', () => {
    test.beforeEach(async ({ page }) => {
        await page.goto('/');
        await expect(page.getByRole('heading', { name: 'BigQuery Explorer' })).toBeVisible();
    });

    test('lists projects in the sidebar', async ({ page }) => {
        await expect(page.getByTestId('project-local-project')).toBeVisible();
        await expect(page.getByTestId('project-local-project')).toContainText('local-project');
    });

    test('navigates the resource tree and opens a table resource tab', async ({ page }) => {
        await selectTable(page);
        await expect(page.getByTestId('breadcrumbs')).toContainText('local-project');
        await expect(page.getByTestId('breadcrumbs')).toContainText('table_a');
        await expect(page.getByTestId('table-tab-page')).toBeVisible();
        await expect(page.getByTestId('table-tab-schema')).toBeVisible();
    });

    test('table Schema tab lists fields and Copy as JSON works', async ({ page, context }) => {
        await context.grantPermissions(['clipboard-read', 'clipboard-write']);
        await openTableSchemaTab(page);
        await expect(page.getByTestId('schema-field-id')).toBeVisible();
        await expect(page.getByTestId('schema-field-name')).toBeVisible();

        const copyMenu = page.getByTestId('table-tab-schema').locator('button[aria-haspopup="menu"]');
        await copyMenu.click();
        await page.getByRole('menuitem', { name: 'Copy as JSON' }).click();

        const copied = await page.evaluate(async () => navigator.clipboard.readText());
        const parsed = JSON.parse(copied) as { name: string }[];
        expect(parsed.some((field) => field.name === 'id')).toBe(true);
        expect(parsed.some((field) => field.name === 'name')).toBe(true);
    });

    test('table Details tab shows info and storage', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('table-resource-tab-details').click();
        const details = page.getByTestId('table-tab-details');
        await expect(details).toBeVisible();
        await expect(details).toContainText('Table ID');
        await expect(details).toContainText('table_a');
        await expect(page.getByTestId('table-storage-info')).toBeVisible();
        await expect(details).toContainText('Number of rows');
    });

    test('table Preview tab shows paginated rows or fallback note', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('table-resource-tab-preview').click();
        await expect(page.getByTestId('table-tab-preview')).toBeVisible();
        const fallback = page.getByTestId('table-preview-fallback-note');
        const alice = page.getByRole('cell', { name: 'alice' });
        await expect(fallback.or(alice)).toBeVisible();
        if (await alice.isVisible()) {
            await expect(page.getByRole('cell', { name: 'bob' })).toBeVisible();
            await expect(page.getByTestId('table-preview-page-size')).toBeVisible();
        }
    });

    test('table Query toolbar opens a persisted query tab', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('open-query-from-table').click();
        await expect(page.getByTestId('sql-editor')).toBeVisible();
        const editor = page.getByTestId('sql-editor').locator('.cm-content');
        await expect(editor).toContainText('SELECT');
        await expect(editor).toContainText('table_a');
        await expect(editor).toContainText('LIMIT 1000');
        await expect(page.getByRole('tab', { name: 'table_a' })).toHaveCount(2);
    });

    test('creates an empty table with schema and shows it in Overview and sidebar', async ({ page }) => {
        await openDataset(page);
        await page.getByTestId('create-table-button').click();
        await expect(page.getByTestId('create-table-modal')).toBeVisible();

        await page.getByTestId('create-table-name').fill('e2e_created_table');

        const fieldInputs = page.getByTestId('schema-builder').locator('input[placeholder="Field name"]');
        await fieldInputs.nth(0).fill('event_id');
        await fieldInputs.nth(1).fill('event_name');

        const typeSelects = page.getByTestId('schema-builder').locator('select').filter({ hasText: 'STRING' });
        await typeSelects.first().selectOption('INT64');
        await typeSelects.nth(1).selectOption('STRING');

        const modeSelects = page.getByTestId('schema-builder').locator('select').filter({ hasText: 'NULLABLE' });
        await modeSelects.first().selectOption('REQUIRED');

        await Promise.all([
            page.waitForResponse(
                (r) =>
                    r.url().includes('/datasets/test-dataset/tables') &&
                    r.request().method() === 'POST' &&
                    r.ok(),
            ),
            page.getByTestId('create-table-submit').click(),
        ]);

        await expect(page.getByTestId('create-table-modal')).not.toBeVisible();
        await expect(page.getByTestId('dataset-overview-tables')).toContainText('e2e_created_table');

        await expect(page.getByTestId('table-e2e_created_table')).toBeVisible({ timeout: 10_000 });
    });

    test('opens a dataset from the sidebar and shows Overview', async ({ page }) => {
        await openDataset(page);
        await expect(page.getByTestId('breadcrumbs')).toContainText('test-dataset');
        await expect(page.getByTestId('dataset-overview-tables')).toBeVisible();
        await expect(page.getByRole('link', { name: 'table_a' })).toBeVisible();
    });

    test('dataset Details tab shows dataset info fields', async ({ page }) => {
        await openDataset(page);
        await page.getByTestId('dataset-resource-tab-details').click();
        const details = page.getByTestId('dataset-tab-details');
        await expect(details).toBeVisible();
        await expect(details).toContainText('Dataset ID');
        await expect(details).toContainText('Data location');
        await expect(details).toContainText('test-dataset');
    });

    test('dataset Insights tab shows Unplanned placeholder', async ({ page }) => {
        await openDataset(page);
        await page.getByTestId('dataset-resource-tab-insights').click();
        await expect(page.getByTestId('dataset-tab-insights')).toBeVisible();
        await expect(page.getByText('Dataset insights are not planned yet.')).toBeVisible();
    });

    test('runs a query and shows results', async ({ page }) => {
        await openQueryFromTable(page);
        await page.getByTestId('run-query').click();
        await page.waitForResponse((r) => r.url().includes('/queries') && r.ok());
        await expect(page.getByTestId('results-tab')).toHaveClass(/border-blue-500/);
        await expect(page.getByTestId('results-tab')).toContainText('(2)');
        await expect(page.getByRole('cell', { name: 'alice' })).toBeVisible();
        await expect(page.getByRole('cell', { name: 'bob' })).toBeVisible();
    });

    test('formats SQL without error', async ({ page }) => {
        await openQueryFromTable(page);
        const editor = page.getByTestId('sql-editor').locator('.cm-content');
        const before = (await editor.textContent()) ?? '';
        await page.getByRole('button', { name: 'Format SQL' }).click();
        const after = (await editor.textContent()) ?? '';
        expect(after.length).toBeGreaterThan(0);
        expect(after).not.toBe(before);
    });

    test('share URL restores selection and query', async ({ page, context }) => {
        await context.grantPermissions(['clipboard-read', 'clipboard-write']);
        await openQueryFromTable(page);
        await page.getByRole('button', { name: 'Share' }).click();
        const sharedUrl = await page.evaluate(async () => navigator.clipboard.readText());
        expect(sharedUrl).toContain('project=local-project');
        expect(sharedUrl).toContain('dataset=test-dataset');
        expect(sharedUrl).toContain('table=table_a');

        const fresh = await context.newPage();
        await fresh.goto(sharedUrl);
        await expect(fresh.getByTestId('project-local-project')).toBeVisible();
        await expect(fresh.getByTestId('sql-editor')).toBeVisible();
        const editor = fresh.getByTestId('sql-editor').locator('.cm-content');
        await expect(editor).toContainText('SELECT');
        await expect(editor).toContainText('table_a');
    });

    test('opens multiple tabs and restores session after reload', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('new-query-tab').click();
        await expect(page.getByRole('tab', { name: 'table_a' })).toBeVisible();
        await expect(page.getByRole('tab', { name: 'Untitled query' })).toBeVisible();

        await page.reload();
        await expect(page.getByRole('tab', { name: 'table_a' })).toBeVisible();
        await expect(page.getByRole('tab', { name: 'Untitled query' })).toBeVisible();
    });
});
