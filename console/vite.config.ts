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
