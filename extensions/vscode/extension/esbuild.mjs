import * as fs from 'node:fs';
import * as path from 'node:path';

import * as esbuild from 'esbuild';

const root = path.dirname(new URL(import.meta.url).pathname);
const serverDist = path.join(root, '..', 'server', 'dist');
const extensionServerDist = path.join(root, 'server', 'dist');

await esbuild.build({
  entryPoints: ['src/extension.ts'],
  bundle: true,
  platform: 'node',
  format: 'cjs',
  outfile: 'dist/extension.js',
  external: ['vscode'],
  sourcemap: true,
  target: 'node18',
});

fs.mkdirSync(extensionServerDist, { recursive: true });
for (const file of fs.readdirSync(serverDist)) {
  fs.copyFileSync(path.join(serverDist, file), path.join(extensionServerDist, file));
}
