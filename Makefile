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
