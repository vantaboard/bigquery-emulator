import { defineConfig, devices } from '@playwright/test';

const isCI = Boolean(process.env.CI);
const isLocalEmulator = process.env.E2E_LOCAL === '1';

export default defineConfig({
    testDir: './e2e',
    testMatch: '**/*.spec.ts',
    fullyParallel: true,
    forbidOnly: isCI,
    retries: isCI ? 1 : 0,
    workers: 1,
    reporter: isCI ? [['list'], ['html', { open: 'never' }]] : 'list',
    use: {
        baseURL: process.env.PLAYWRIGHT_BASE_URL ?? 'http://127.0.0.1:8080',
        trace: 'on-first-retry',
        ...devices['Desktop Chrome'],
    },
    globalSetup: isLocalEmulator ? undefined : './e2e/global-setup.ts',
    globalTeardown: isLocalEmulator ? undefined : './e2e/global-teardown.ts',
});
