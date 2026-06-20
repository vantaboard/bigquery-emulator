/// <reference types="vite/client" />

interface ImportMetaEnv {
    readonly VITE_API_URL: string | undefined;
    readonly VITE_PROXY_TARGET: string | undefined;
    readonly VITE_DEV_PORT: string | undefined;
    readonly VITE_DEFAULT_PROJECT: string | undefined;
    readonly VITE_ALLOW_EMULATOR_PROJECT_ADMIN: string | undefined;
    readonly VITE_SQL_TOOLS_TOKEN: string | undefined;
}

interface ImportMeta {
    readonly env: ImportMetaEnv;
}
