import path from 'node:path';
import tailwindcss from '@tailwindcss/vite';
import react from '@vitejs/plugin-react';
import { defineConfig, loadEnv } from 'vite';

export default defineConfig(({ mode }) => {
    const env = loadEnv(mode, process.cwd(), '');
    const proxyTarget = env.VITE_PROXY_TARGET ?? 'http://127.0.0.1:9050';

    return {
        plugins: [react(), tailwindcss()],
        resolve: {
            alias: {
                '@': path.resolve(__dirname, 'src'),
            },
            // CodeMirror breaks instanceof checks when more than one copy of
            // its core packages is loaded. Force a single instance so the
            // editor's extension set resolves correctly.
            dedupe: ['@codemirror/state', '@codemirror/view'],
        },
        // Pre-bundle the CodeMirror packages together so they share a single
        // @codemirror/state instance. Without this, @uiw/react-codemirror and
        // the language/extension packages can be optimized into separate chunks
        // that each carry their own copy, breaking instanceof checks.
        optimizeDeps: {
            include: [
                '@uiw/react-codemirror',
                '@codemirror/state',
                '@codemirror/view',
                '@codemirror/autocomplete',
                '@codemirror/lint',
                '@codemirror/lang-sql',
                '@codemirror/lang-json',
            ],
        },
        server: {
            port: Number(env.VITE_DEV_PORT ?? 5173),
            proxy: {
                '/bigquery': {
                    target: proxyTarget,
                    changeOrigin: true,
                },
                '/api/emulator': {
                    target: proxyTarget,
                    changeOrigin: true,
                },
            },
        },
        test: {
            globals: false,
            environment: 'node',
            include: ['src/**/*.test.ts'],
        },
    };
});
