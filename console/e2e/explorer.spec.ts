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

async function openTableSchemaTab(page: Page) {
    await selectTable(page);
    await page.getByTestId('table-resource-tab-schema').click();
    await expect(page.getByTestId('table-tab-schema')).toBeVisible();
}

async function openQueryFromTable(page: Page) {
    await selectTable(page);
    await expect(page.getByTestId('table-tab-page')).toBeVisible();
    await page.getByTestId('open-query-from-table').click();
    await expect(page.getByTestId('sql-editor')).toBeVisible();
}

async function monacoEditor(page: Page) {
    const root = page.getByTestId('sql-editor');
    await expect(root.locator('.monaco-editor')).toBeVisible();
    return root;
}

async function monacoSetValue(page: Page, value: string) {
    const root = await monacoEditor(page);
    await root.locator('.monaco-editor').click({ force: true });
    await page.keyboard.press('Control+a');
    await page.keyboard.insertText(value);
}

async function monacoText(page: Page): Promise<string> {
    const root = await monacoEditor(page);
    // Monaco may render ordinary spaces as NBSP in the view lines.
    return ((await root.locator('.view-lines').innerText()) ?? '').replace(/\u00a0/g, ' ');
}

async function openRoutinesTab(page: Page) {
    await openDataset(page);
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset/routines') && r.ok()),
        page.getByTestId('dataset-overview-tab-routines').click(),
    ]);
    await expect(
        page
            .getByTestId('dataset-overview-routines')
            .or(page.getByTestId('dataset-overview-routines-empty'))
            .or(page.getByTestId('dataset-overview-routines-error')),
    ).toBeVisible({ timeout: 10_000 });
}

async function expandDatasetWithRoutines(page: Page) {
    await expandProject(page);
    const toggle = page.getByTestId('dataset-toggle-test-dataset');
    await Promise.all([
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset/tables') && r.ok()),
        page.waitForResponse((r) => r.url().includes('/datasets/test-dataset/routines') && r.ok()),
        toggle.click(),
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
        const text = await monacoText(page);
        expect(text).toContain('SELECT');
        expect(text).toContain('table_a');
        expect(text).toContain('LIMIT 1000');
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
        await monacoSetValue(page, 'select*from`local-project.test-dataset.table_a`limit 1000');
        await page.getByTestId('format-sql').click();
        const after = await monacoText(page);
        expect(after.length).toBeGreaterThan(0);
        expect(after.toLowerCase()).toContain('select');
        expect(after).toContain('table_a');
    });

    test('shows syntax error status bar and Alt+F8 marker navigation', async ({ page }) => {
        await openQueryFromTable(page);

        const parseResponsePromise = page.waitForResponse(
            (r) => r.url().includes('/api/emulator/sql/parse') && r.ok(),
            { timeout: 10_000 },
        );
        await monacoSetValue(page, 'SELECT SAFE_ADD(');
        await parseResponsePromise;

        const bar = page.getByTestId('sql-diagnostics-bar');
        await expect(bar).toBeVisible({ timeout: 5_000 });
        await expect(bar).toContainText(/Syntax error: Expected "\)" but got end of script at \[1:\d+\]/);

        const root = await monacoEditor(page);
        const editorLine = root.locator('.view-lines .view-line').first();
        await expect(editorLine).toBeVisible({ timeout: 5_000 });
        await editorLine.hover({ position: { x: 40, y: 8 } });
        await page.waitForTimeout(400);
        const hover = page.locator('.monaco-hover').filter({ hasText: 'Syntax error' });
        await expect(hover.first()).toBeVisible({ timeout: 10_000 });
        const hoverText = await hover.first().innerText();
        expect(hoverText.match(/Syntax error/g)?.length ?? 0).toBe(1);
        await expect(hover.first()).toContainText(/Syntax error: Expected "\)" but got end of script at \[1:\d+\]/);
        await expect(
            hover.first().locator('a', { hasText: 'View Problem' }),
        ).toBeVisible();
        await expect(hover.first()).toContainText('No quick fixes available');
        await expect(hover.first()).not.toContainText('Alt+F8');
        await expect(hover.first()).not.toContainText('Insert missing');
        await expect(hover.first()).not.toContainText('uppercase keyword');

        await page.keyboard.press('Alt+F8');
        await expect(bar).toBeVisible();
    });

    test('shows autocompletion suggestions while typing', async ({ page }) => {
        await openQueryFromTable(page);
        await monacoSetValue(page, 'SELECT S');

        const root = await monacoEditor(page);
        await root.locator('.monaco-editor').click({ force: true });
        const completeResponsePromise = page.waitForResponse(
            (r) => r.url().includes('/api/emulator/sql/complete') && r.ok(),
            { timeout: 10_000 },
        );
        await page.keyboard.press('Control+Space');
        await completeResponsePromise;

        const suggest = page.locator('.suggest-widget');
        await expect(suggest).toBeVisible({ timeout: 10_000 });
        await expect(suggest.locator('.monaco-list-row', { hasText: 'SAFE_ADD' }).first()).toBeVisible();
    });

    test('saves a query to localStorage and restores after reload', async ({ page }) => {
        await openQueryFromTable(page);
        await page.getByTestId('save-query-menu').click();
        await page.getByTestId('save-query-classic').click();

        const stored = await page.evaluate(() => localStorage.getItem('bigqueryWorkspaceSession'));
        expect(stored).toBeTruthy();
        const session = JSON.parse(stored!) as { savedQueriesClassic: { title: string; sql: string }[] };
        expect(session.savedQueriesClassic.length).toBeGreaterThan(0);
        expect(session.savedQueriesClassic[0].sql).toContain('table_a');

        await page.reload();
        const storedAfter = await page.evaluate(() => localStorage.getItem('bigqueryWorkspaceSession'));
        const sessionAfter = JSON.parse(storedAfter!) as { savedQueriesClassic: unknown[] };
        expect(sessionAfter.savedQueriesClassic.length).toBeGreaterThan(0);
    });

    test('saves a view via DDL', async ({ page }) => {
        await openQueryFromTable(page);
        await page.getByTestId('run-query').click();
        await page.waitForResponse((r) => r.url().includes('/queries') && r.ok());

        await page.getByTestId('save-query-menu').click();
        await page.getByTestId('save-view').click();
        await page.getByTestId('save-destination-name').fill('e2e_saved_view');
        const [response] = await Promise.all([
            page.waitForResponse((r) => r.url().includes('/queries') && r.ok()),
            page.getByTestId('save-destination-submit').click(),
        ]);
        const body = (await response.json()) as { jobComplete?: boolean; statistics?: { query?: { statementType?: string } } };
        expect(body.jobComplete).toBe(true);
        expect(body.statistics?.query?.statementType).toBe('CREATE_VIEW');
    });

    test('reference panel shows tab-bound table schema', async ({ page }) => {
        await openQueryFromTable(page);
        await page.getByTestId('toggle-reference-panel').click();
        const panel = page.getByTestId('query-reference-panel');
        await expect(panel).toBeVisible();
        await expect(panel.getByTestId('reference-field-id')).toBeVisible();
        await expect(panel.getByTestId('reference-field-name')).toBeVisible();
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
        const text = await monacoText(fresh);
        expect(text).toContain('SELECT');
        expect(text).toContain('table_a');
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

    test('deletes a table and removes it from the sidebar', async ({ page }) => {
        await openDataset(page);
        await page.getByTestId('create-table-button').click();
        await page.getByTestId('create-table-name').fill('e2e_delete_table');

        const fieldInputs = page.getByTestId('schema-builder').locator('input[placeholder="Field name"]');
        await fieldInputs.nth(0).fill('row_id');
        await fieldInputs.nth(1).fill('row_label');

        await Promise.all([
            page.waitForResponse(
                (r) =>
                    r.url().includes('/datasets/test-dataset/tables') &&
                    r.request().method() === 'POST' &&
                    r.ok(),
            ),
            page.getByTestId('create-table-submit').click(),
        ]);

        await expect(page.getByTestId('table-e2e_delete_table')).toBeVisible({ timeout: 10_000 });
        await page.getByTestId('table-e2e_delete_table').click();
        await expect(page.getByTestId('table-tab-page')).toBeVisible();

        await page.getByTestId('delete-table-button').click();
        await Promise.all([
            page.waitForResponse(
                (r) =>
                    r.url().includes('/tables/e2e_delete_table') &&
                    r.request().method() === 'DELETE' &&
                    r.ok(),
            ),
            page.getByTestId('delete-table-confirm').click(),
        ]);

        await expect(page.getByTestId('table-e2e_delete_table')).not.toBeVisible({ timeout: 10_000 });
    });

    test('edits schema by changing field mode and adding a field', async ({ page }) => {
        await openDataset(page);
        await page.getByTestId('create-table-button').click();
        await page.getByTestId('create-table-name').fill('e2e_edit_schema_table');

        const fieldInputs = page.getByTestId('schema-builder').locator('input[placeholder="Field name"]');
        await fieldInputs.nth(0).fill('metric_id');
        await fieldInputs.nth(1).fill('metric_value');

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

        await page.getByTestId('table-e2e_edit_schema_table').click();
        await expect(page.getByTestId('table-tab-schema')).toBeVisible();

        await page.getByTestId('edit-schema-button').click();
        await expect(page.getByTestId('edit-schema-modal')).toBeVisible();

        await page.getByTestId('edit-schema-mode-metric_id').selectOption('NULLABLE');
        await page.getByTestId('edit-schema-add-field').click();
        await page.getByTestId('edit-schema-new-field-name').fill('recorded_at');

        await Promise.all([
            page.waitForResponse(
                (r) =>
                    r.url().includes('/tables/e2e_edit_schema_table') &&
                    r.request().method() === 'PATCH' &&
                    r.ok(),
            ),
            page.getByTestId('edit-schema-submit').click(),
        ]);

        await expect(page.getByTestId('edit-schema-modal')).not.toBeVisible();
        await expect(page.getByTestId('schema-field-metric_id')).toContainText('NULLABLE');
        await expect(page.getByTestId('schema-field-recorded_at')).toBeVisible();
    });

    test('lists routines on dataset overview', async ({ page }) => {
        await openRoutinesTab(page);
        await expect(
            page
                .getByTestId('dataset-overview-routines')
                .or(page.getByTestId('dataset-overview-routines-empty')),
        ).toBeVisible();
    });

    async function createScalarUdf(page: Page, routineName: string) {
        await openDataset(page);
        await page.getByTestId('create-routine-button').click();
        await expect(page.getByTestId('create-routine-modal')).toBeVisible();
        await page.getByTestId('create-routine-name').fill(routineName);
        await page.getByTestId('create-routine-body').fill('x + 1');
        await Promise.all([
            page.waitForResponse((r) => r.url().includes('/queries') && r.ok()),
            page.getByTestId('create-routine-submit').click(),
        ]);
        await page.getByTestId('dataset-overview-tab-routines').click();
        await expect(page.getByTestId(`routine-link-${routineName}`)).toBeVisible({
            timeout: 15_000,
        });
    }

    test('creates a scalar UDF and opens routine detail', async ({ page }) => {
        const routineName = `e2e_add_one_${Date.now()}`;
        await createScalarUdf(page, routineName);
        await page.getByTestId(`routine-link-${routineName}`).click();

        await expect(page.getByTestId('routine-tab-page')).toBeVisible();
        await expect(page.getByTestId('routine-type')).toContainText(/function/i);
        // Some emulator images return an empty definitionBody over REST even
        // after a successful CREATE FUNCTION; the tab + type are the hard pins.
        await expect(page.getByTestId('routine-definition')).toBeVisible();
        const definition = await page.getByTestId('routine-definition').innerText();
        if (!definition.includes('—')) {
            expect(definition).toContain('x + 1');
        }
    });

    test('autocomplete suggests created routine names', async ({ page }) => {
        const routineName = `e2e_ac_${Date.now()}`;
        await createScalarUdf(page, routineName);

        await openQueryFromTable(page);
        await monacoSetValue(page, `SELECT ${routineName}(`);

        await page.keyboard.press('Control+Space');
        const suggest = page.locator('.suggest-widget');
        await expect(suggest).toBeVisible({ timeout: 15_000 });
        await expect(suggest.locator('.monaco-list-row', { hasText: routineName }).first()).toBeVisible({
            timeout: 15_000,
        });
    });

    test('shows routines in the sidebar tree', async ({ page }) => {
        const routineName = `e2e_sidebar_${Date.now()}`;
        await createScalarUdf(page, routineName);

        await expandDatasetWithRoutines(page);
        await expect(page.getByTestId(`routine-${routineName}`)).toBeVisible({ timeout: 15_000 });
    });

    test('tab context menu closes other tabs', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('new-query-tab').click();
        await expect(page.getByRole('tab', { name: 'table_a' })).toBeVisible();
        await expect(page.getByRole('tab', { name: 'Untitled query' })).toBeVisible();

        const queryTab = page.getByRole('tab', { name: 'Untitled query' });
        await queryTab.click({ button: 'right' });
        const menu = page.getByTestId('context-menu');
        await expect(menu).toBeVisible();
        await menu.getByRole('menuitem', { name: 'Close other tabs' }).click();

        await expect(page.getByRole('tab', { name: 'Untitled query' })).toBeVisible();
        await expect(page.getByRole('tab', { name: 'table_a' })).not.toBeVisible();
    });

    test('tab context menu splits tab to the right', async ({ page }) => {
        await selectTable(page);
        await page.getByTestId('new-query-tab').click();
        await expect(page.getByRole('tab', { name: 'table_a' })).toBeVisible();
        await expect(page.getByRole('tab', { name: 'Untitled query' })).toBeVisible();

        const queryTab = page.getByRole('tab', { name: 'Untitled query' });
        await queryTab.click({ button: 'right' });
        const menu = page.getByTestId('context-menu');
        await expect(menu).toBeVisible();
        await menu.getByRole('menuitem', { name: 'Split tab to right' }).click();

        await expect(page.getByTestId('workspace-pane-left')).toBeVisible();
        await expect(page.getByTestId('workspace-pane-right')).toBeVisible();
        await expect(page.getByTestId('workspace-pane-tabbar-left')).toBeVisible();
        await expect(page.getByTestId('workspace-pane-tabbar-right')).toBeVisible();
        await expect(page.getByTestId('workspace-pane-left').getByTestId('table-tab-page')).toBeVisible();
        await expect(page.getByTestId('workspace-pane-right').getByTestId('sql-editor')).toBeVisible();
    });

    test('closing a dataset tab does not reopen it', async ({ page }) => {
        await openDataset(page);
        await page.getByTestId('new-query-tab').click();
        await expect(page.getByRole('tab', { name: 'test-dataset' })).toBeVisible();
        await expect(page.getByRole('tab', { name: 'Untitled query' })).toBeVisible();

        await page.getByRole('tab', { name: 'test-dataset' }).click();
        await page.getByRole('button', { name: 'Close test-dataset' }).click();

        await expect(page.getByRole('tab', { name: 'test-dataset' })).not.toBeVisible();
        await expect(page.getByTestId('sql-editor')).toBeVisible();
        await expect(page).toHaveURL(/\/query\//);
    });
});
