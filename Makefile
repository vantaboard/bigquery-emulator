# Makefile for the BigQuery emulator. Mirrors Taskfile.yml for users who
# don't have `task` installed.
#
# Gateway-only: the C++ engine builds exclusively via Bazel. Use
# `task emulator:build-engine-bazel` (or `bazel build
# //binaries/emulator_main:emulator_main` directly) to produce
# `bin/emulator_main` + `bin/libduckdb.so`.

BIN_DIR  := bin
HTTP_PORT ?= 9050

.PHONY: all build run test vet fmt lint clean proto proto-install proto-lint

all: build

build:
	@mkdir -p $(BIN_DIR)
	go build -o $(BIN_DIR)/gateway_main ./binaries/gateway_main

run: build
	$(BIN_DIR)/gateway_main --engine_binary= --http_port=$(HTTP_PORT) --log_requests

test:
	go test ./...

vet:
	go vet ./...

fmt:
	gofmt -s -w .

lint:
	@test -z "$$(gofmt -s -l .)" || (echo 'gofmt would reformat:' && gofmt -s -l . && exit 1)
	go vet ./...

proto-install:
	go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
	go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest

proto: proto-install
	buf generate

proto-lint:
	buf lint

clean:
	rm -rf $(BIN_DIR)
