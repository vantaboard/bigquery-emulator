import { expect, test, type Page } from '@playwright/test';

async function selectTable(page: Page) {
    const project = page.getByTestId('project-local-project');
    await expect(project).toBeVisible();
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/bigquery/v2/projects/local-project/datasets') && r.ok()),
        project.click(),
    ]);
    const dataset = page.getByTestId('dataset-test-dataset');
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset/tables') && r.ok()),
        dataset.click(),
    ]);
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
