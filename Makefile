VERSION ?= latest
REVISION := $(shell git rev-parse --short HEAD 2>/dev/null || echo unknown)
LINKED_GOWORK := $(CURDIR)/go.work.linked

emulator/build:
	CGO_ENABLED=1 CXX=clang++ go build -o bigquery-emulator \
		./cmd/bigquery-emulator

emulator/build-linked:
	CGO_ENABLED=1 CXX=clang++ GOWORK=$(LINKED_GOWORK) go build -o bigquery-emulator \
		./cmd/bigquery-emulator

docker/build:
	docker build -t bigquery-emulator . \
		--build-arg VERSION=${VERSION}

docker/build-linked:
	docker build -t bigquery-emulator:linked -f Dockerfile.linked . \
		--build-arg VERSION=${VERSION} \
		--build-context go_zetasql=../go-zetasql \
		--build-context go_zetasqlite=../go-zetasqlite
