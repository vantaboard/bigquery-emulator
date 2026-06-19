import { execSync } from 'node:child_process';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const composeFile = path.join(root, 'docker-compose.e2e.yaml');
const waitScript = path.join(root, 'e2e/scripts/wait-for-stack.sh');

function run(cmd: string) {
    execSync(cmd, { cwd: root, stdio: 'inherit' });
}

export default async function globalSetup() {
    try {
        run(`docker compose -f "${composeFile}" up -d --wait --build`);
        run(`bash "${waitScript}"`);
    } catch (err) {
        try {
            run(`docker compose -f "${composeFile}" logs bigquery`);
            run(`docker compose -f "${composeFile}" logs explorer-api`);
        } catch {
            /* ignore log failure */
        }
        throw err;
    }
}
