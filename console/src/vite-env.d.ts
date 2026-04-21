/// <reference types="vite/client" />

interface ImportMetaEnv {
    readonly VITE_API_URL: string | undefined;
    readonly VITE_PROXY_TARGET: string | undefined;
    readonly VITE_DEV_PORT: string | undefined;
}

interface ImportMeta {
    readonly env: ImportMetaEnv;
}
