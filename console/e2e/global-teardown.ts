import { execSync } from 'node:child_process';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const composeFile = path.join(root, 'docker-compose.e2e.yaml');

export default async function globalTeardown() {
    if (process.env.E2E_SKIP_TEARDOWN === '1') {
        return;
    }
    execSync(`docker compose -f "${composeFile}" down -v`, { cwd: root, stdio: 'inherit' });
}
