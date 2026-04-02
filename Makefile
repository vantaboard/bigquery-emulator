VERSION ?= latest
REVISION := $(shell git rev-parse --short HEAD 2>/dev/null || echo unknown)
LINKED_GOWORK := $(CURDIR)/go.work.linked
# Pin matches the default in Dockerfile. Override for local upgrade testing, e.g. GO_ZETASQL_BASE=go-zetasql:dev
GO_ZETASQL_BASE ?= ghcr.io/recidiviz/go-zetasql:0.5.5-recidiviz.3

emulator/build:
	CGO_ENABLED=1 CXX=clang++ go build -o bigquery-emulator \
		./cmd/bigquery-emulator

emulator/build-linked:
	CGO_ENABLED=1 CXX=clang++ GOWORK=$(LINKED_GOWORK) go build -o bigquery-emulator \
		./cmd/bigquery-emulator

docker/build:
	docker build -t bigquery-emulator . \
		--build-arg VERSION=${VERSION} \
		--build-arg GO_ZETASQL_BASE=${GO_ZETASQL_BASE}

docker/build-linked:
	docker build -t bigquery-emulator:linked -f Dockerfile.linked . \
		--build-arg VERSION=${VERSION} \
		--build-arg GO_ZETASQL_BASE=${GO_ZETASQL_BASE} \
		--build-context go_zetasql=../go-zetasql \
		--build-context go_zetasqlite=../go-zetasqlite

# Same dev image + GO_CACHE_ROOT as ../go-zetasql. Requires sibling checkouts and go.work.linked (build go-zetasql:dev in go-zetasql first).
GO_ZETASQL_ROOT ?= $(abspath $(CURDIR)/../go-zetasql)
GO_ZETASQLITE_ROOT ?= $(abspath $(CURDIR)/../go-zetasqlite)
DOCKER_DEV_IMAGE ?= go-zetasql:dev
GO_CACHE_ROOT ?= $(HOME)/.cache/go-zetasql

.PHONY: test/linux
test/linux:
	docker run --rm \
		-e CGO_ENABLED=1 -e CC=clang -e CXX=clang++ \
		-e GOPROXY=https://proxy.golang.org,direct \
		-e GOSUMDB=sum.golang.org \
		-e CCACHE_DIR=/root/.ccache -e CCACHE_COMPRESS=1 \
		-e GOWORK=/work/bigquery-emulator/go.work.linked \
		-v "$(CURDIR)":/work/bigquery-emulator \
		-v "$(GO_ZETASQL_ROOT)":/work/go-zetasql \
		-v "$(GO_ZETASQLITE_ROOT)":/work/go-zetasqlite \
		-v "$(GO_CACHE_ROOT)/gocache":/root/.cache/go-build \
		-v "$(GO_CACHE_ROOT)/gomodcache":/go/pkg/mod \
		-v "$(GO_CACHE_ROOT)/ccache":/root/.ccache \
		-w /work/bigquery-emulator \
		$(DOCKER_DEV_IMAGE) \
		bash -c "go test -race -v ./... -count=1"
