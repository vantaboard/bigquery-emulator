# Makefile for the BigQuery emulator. Mirrors Taskfile.yml for users who
# don't have `task` installed.

BIN_DIR  := bin
HTTP_PORT ?= 9050

.PHONY: all build build-engine run run-full test vet fmt lint clean

all: build

build:
	@mkdir -p $(BIN_DIR)
	go build -o $(BIN_DIR)/gateway_main ./binaries/gateway_main

build-engine:
	cmake -S . -B build-out -DCMAKE_BUILD_TYPE=Release
	cmake --build build-out --target emulator_main -j

run: build
	$(BIN_DIR)/gateway_main --engine_binary= --http_port=$(HTTP_PORT) --log_requests

run-full: build build-engine
	cp build-out/emulator_main $(BIN_DIR)/emulator_main
	$(BIN_DIR)/gateway_main --engine_binary=$(BIN_DIR)/emulator_main --http_port=$(HTTP_PORT) --log_requests

test:
	go test ./...

vet:
	go vet ./...

fmt:
	gofmt -s -w .

lint:
	@test -z "$$(gofmt -s -l .)" || (echo 'gofmt would reformat:' && gofmt -s -l . && exit 1)
	go vet ./...

clean:
	rm -rf $(BIN_DIR) build-out
