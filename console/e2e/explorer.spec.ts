import { expect, test, type Page } from '@playwright/test';

async function selectTable(page: Page) {
    const project = page.getByTestId('project-local-project');
    await expect(project).toBeVisible();
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/api/projects/local-project/datasets') && r.ok()),
        project.click(),
    ]);
    const dataset = page.getByTestId('dataset-test-dataset');
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset/tables') && r.ok()),
        dataset.click(),
    ]);
    const table = page.getByTestId('table-table_a');
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/tables/table_a/schema') && r.ok()),
        table.click(),
    ]);
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

    test('navigates the resource tree and shows table metadata', async ({ page }) => {
        await selectTable(page);
        await expect(page.getByRole('button', { name: 'Table info' })).toHaveClass(/border-blue-500/);
        await expect(page.getByRole('cell', { name: 'local-project.test-dataset.table_a' })).toBeVisible();
        await expect(page.getByText('Rows', { exact: true })).toBeVisible();
    });

    test('runs a query and shows results', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('run-query').click();
        await page.waitForResponse((r) => r.url().includes('/api/query') && r.ok());
        await expect(page.getByTestId('results-tab')).toHaveClass(/border-blue-500/);
        await expect(page.getByTestId('results-tab')).toContainText('(2)');
        await expect(page.getByRole('cell', { name: 'alice' })).toBeVisible();
        await expect(page.getByRole('cell', { name: 'bob' })).toBeVisible();
    });

    test('formats SQL without error', async ({ page }) => {
        await selectTable(page);
        const editor = page.getByTestId('sql-editor').locator('.cm-content');
        const before = (await editor.textContent()) ?? '';
        await page.getByRole('button', { name: 'Format SQL' }).click();
        const after = (await editor.textContent()) ?? '';
        expect(after.length).toBeGreaterThan(0);
        expect(after).not.toBe(before);
    });

    test('share URL restores selection and query', async ({ page, context }) => {
        await context.grantPermissions(['clipboard-read', 'clipboard-write']);
        await selectTable(page);
        await page.getByRole('button', { name: 'Share' }).click();
        const sharedUrl = await page.evaluate(async () => navigator.clipboard.readText());
        expect(sharedUrl).toContain('project=local-project');
        expect(sharedUrl).toContain('dataset=test-dataset');
        expect(sharedUrl).toContain('table=table_a');

        const fresh = await context.newPage();
        await fresh.goto(sharedUrl);
        await expect(fresh.getByTestId('project-local-project')).toBeVisible();
        await expect(fresh.getByRole('cell', { name: 'local-project.test-dataset.table_a' })).toBeVisible();
        const editor = fresh.getByTestId('sql-editor').locator('.cm-content');
        await expect(editor).toContainText('SELECT');
        await expect(editor).toContainText('table_a');
    });
});
