import * as esbuild from 'esbuild';

await esbuild.build({
  entryPoints: ['src/server.node.ts'],
  bundle: true,
  platform: 'node',
  format: 'cjs',
  outfile: 'dist/server.js',
  external: ['vscode'],
  sourcemap: true,
  target: 'node18',
});

await esbuild.build({
  entryPoints: ['src/server.browser.ts'],
  bundle: true,
  platform: 'browser',
  format: 'esm',
  outfile: 'dist/server.browser.js',
  sourcemap: true,
  target: 'es2022',
  // createServer imports vscode-languageserver/node; remap to browser for this bundle.
  alias: {
    'vscode-languageserver/node.js': 'vscode-languageserver/browser.js',
  },
});
