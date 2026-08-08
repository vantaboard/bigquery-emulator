# Production image: static React app + nginx proxying /bigquery to the emulator.
# Build from the monorepo root so workspace packages resolve:
#   docker build -f console/Dockerfile -t bq-console .
FROM node:22-alpine AS build
WORKDIR /app
RUN corepack enable && corepack prepare pnpm@10.12.1 --activate
ARG VITE_DEFAULT_PROJECT=
ARG VITE_ALLOW_EMULATOR_PROJECT_ADMIN=
COPY pnpm-workspace.yaml package.json pnpm-lock.yaml ./
COPY console/package.json console/
COPY extensions/vscode/shared/package.json extensions/vscode/shared/
COPY extensions/vscode/server/package.json extensions/vscode/server/
COPY extensions/vscode/extension/package.json extensions/vscode/extension/
COPY extensions/vscode/test/package.json extensions/vscode/test/
RUN pnpm install --frozen-lockfile
COPY . .
ENV VITE_API_URL=
ENV VITE_DEFAULT_PROJECT=$VITE_DEFAULT_PROJECT
ENV VITE_ALLOW_EMULATOR_PROJECT_ADMIN=$VITE_ALLOW_EMULATOR_PROJECT_ADMIN
RUN pnpm --filter @bigquery-emulator/vscode-shared build \
 && pnpm --filter @bigquery-emulator/vscode-server build \
 && pnpm --filter @bigquery-emulator/console build

FROM nginx:1.27-alpine
COPY --from=build /app/console/dist /usr/share/nginx/html
COPY console/nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
