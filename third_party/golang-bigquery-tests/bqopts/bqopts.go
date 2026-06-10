// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package bqopts provides optional [google.golang.org/api/option.ClientOption]
// values for [cloud.google.com/go/bigquery.NewClient] when running against a
// local BigQuery emulator (BIGQUERY_EMULATOR_HOST), and for
// [cloud.google.com/go/bigquery/migration/apiv2alpha.NewRESTClient] (Migration REST).
package bqopts

import (
	"net/http"
	"os"
	"strings"

	"google.golang.org/api/option"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

// emulatorAPIRegionRoundTripper adds X-BigQuery-Emulator-Api-Region on each request so the
// emulator can enforce regional dataset rules when Host is loopback.
type emulatorAPIRegionRoundTripper struct {
	Base   http.RoundTripper
	Region string
}

func (t *emulatorAPIRegionRoundTripper) RoundTrip(req *http.Request) (*http.Response, error) {
	base := t.Base
	if base == nil {
		base = http.DefaultTransport
	}
	r := req.Clone(req.Context())
	r.Header.Set("X-BigQuery-Emulator-Api-Region", t.Region)
	return base.RoundTrip(r)
}

// DataTransferRESTClientOptions returns options for
// [cloud.google.com/go/bigquery/datatransfer/apiv1.NewRESTClient] when
// BIGQUERY_EMULATOR_HOST is set. The emulator serves Data Transfer metadata
// routes on the same HTTP host:port as BigQuery Jobs REST.
// When BIGQUERY_EMULATOR_HOST is unset, returns nil.
func DataTransferRESTClientOptions() []option.ClientOption {
	return ClientOptions()
}

// ClientOptions returns client options for [cloud.google.com/go/bigquery.NewClient]
// when the environment variable BIGQUERY_EMULATOR_HOST is set to host:port
// (no URL scheme). The Go client does not read this variable by default.
// When unset, returns nil so callers can spread into NewClient with no effect.
//
// When BIGQUERY_EMULATOR_CLIENT_API_REGION is set (e.g. us-east4), each HTTP request also sends
// X-BigQuery-Emulator-Api-Region so dataset create location checks match a "regional" client
// even though the real Host is 127.0.0.1.
//
// HTTP connections use a bounded dial timeout (default 15s) so tests do not hang when nothing
// is listening; override with BIGQUERY_EMULATOR_HTTP_DIAL_TIMEOUT (Go duration). On dial or
// timeout errors, the returned error includes a hint to start the emulator.
func ClientOptions() []option.ClientOption {
	raw := strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST"))
	if raw == "" {
		return nil
	}
	h := raw
	if !strings.HasPrefix(h, "http://") && !strings.HasPrefix(h, "https://") {
		h = "http://" + h
	}
	reg := strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_CLIENT_API_REGION"))
	return []option.ClientOption{
		option.WithEndpoint(h),
		option.WithoutAuthentication(),
		option.WithHTTPClient(newEmulatorHTTPClient("BIGQUERY_EMULATOR_HOST", raw, reg, emulatorHTTPDialTimeout())),
	}
}

// MigrationRESTClientOptions returns client options for
// [cloud.google.com/go/bigquery/migration/apiv2alpha.NewRESTClient] when running
// against the bigquery-emulator HTTP gateway (BigQuery Migration v2alpha workflow routes).
//
// It reads BIGQUERY_MIGRATION_EMULATOR_HOST as host:port (no URL scheme). If that
// variable is unset but BIGQUERY_EMULATOR_HOST is set, the latter is used so one
// host:port can serve both BigQuery Jobs REST and Migration REST.
// When both are unset, returns nil so callers can spread into NewRESTClient with no effect.
//
// Uses the same HTTP dial timeout and unreachable hints as [ClientOptions] (see
// BIGQUERY_EMULATOR_HTTP_DIAL_TIMEOUT).
func MigrationRESTClientOptions() []option.ClientOption {
	h := strings.TrimSpace(os.Getenv("BIGQUERY_MIGRATION_EMULATOR_HOST"))
	envKey := "BIGQUERY_MIGRATION_EMULATOR_HOST"
	if h == "" {
		h = strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST"))
		envKey = "BIGQUERY_EMULATOR_HOST"
	}
	if h == "" {
		return nil
	}
	rawHost := h
	if !strings.HasPrefix(h, "http://") && !strings.HasPrefix(h, "https://") {
		h = "http://" + h
	}
	return []option.ClientOption{
		option.WithEndpoint(h),
		option.WithoutAuthentication(),
		option.WithHTTPClient(newEmulatorHTTPClient(envKey, rawHost, "", emulatorHTTPDialTimeout())),
	}
}

// BigQueryV2GRPCClientOptions returns options for
// [cloud.google.com/go/bigquery/v2/apiv2_client.NewClient] (BigQuery v2 metadata gRPC) when dialing the
// bigquery-emulator. Reads BIGQUERY_V2_GRPC_ENDPOINT as host:port (no scheme); if unset,
// falls back to BIGQUERY_STORAGE_GRPC_ENDPOINT (the emulator serves BigQuery v2 metadata gRPC on the
// same listener as Storage, Connection, Reservation, and Analytics Hub). Returns nil when neither is set.
//
// Uses BIGQUERY_EMULATOR_GRPC_DIAL_TIMEOUT (default 15s) like [StorageGRPCClientOptions].
func BigQueryV2GRPCClientOptions() []option.ClientOption {
	ep := strings.TrimSpace(os.Getenv("BIGQUERY_V2_GRPC_ENDPOINT"))
	envKey := "BIGQUERY_V2_GRPC_ENDPOINT"
	if ep == "" {
		ep = strings.TrimSpace(os.Getenv("BIGQUERY_STORAGE_GRPC_ENDPOINT"))
		envKey = "BIGQUERY_STORAGE_GRPC_ENDPOINT"
	}
	if ep == "" {
		return nil
	}
	return []option.ClientOption{
		option.WithEndpoint(ep),
		option.WithoutAuthentication(),
		option.WithGRPCDialOption(grpc.WithTransportCredentials(insecure.NewCredentials())),
		option.WithGRPCDialOption(grpc.WithContextDialer(grpcEmulatorDialer(emulatorGRPCDialTimeout(), envKey, ep))),
	}
}

// StorageGRPCClientOptions returns options for BigQuery Storage API gRPC clients
// ([cloud.google.com/go/bigquery/storage/apiv1], [cloud.google.com/go/bigquery/storage/managedwriter], etc.)
// when BIGQUERY_STORAGE_GRPC_ENDPOINT is set to host:port (no URL scheme), as used with the
// bigquery-emulator gRPC listener. When unset, returns nil so callers can spread into client
// constructors with no effect.
//
// gRPC dials use BIGQUERY_EMULATOR_GRPC_DIAL_TIMEOUT (default 15s) so clients fail fast when
// the emulator gRPC listener is down; errors include a hint to start the emulator.
func StorageGRPCClientOptions() []option.ClientOption {
	ep := strings.TrimSpace(os.Getenv("BIGQUERY_STORAGE_GRPC_ENDPOINT"))
	if ep == "" {
		return nil
	}
	return []option.ClientOption{
		option.WithEndpoint(ep),
		option.WithoutAuthentication(),
		option.WithGRPCDialOption(grpc.WithTransportCredentials(insecure.NewCredentials())),
		option.WithGRPCDialOption(grpc.WithContextDialer(grpcEmulatorDialer(emulatorGRPCDialTimeout(), "BIGQUERY_STORAGE_GRPC_ENDPOINT", ep))),
	}
}

// ReservationGRPCClientOptions returns options for
// [cloud.google.com/go/bigquery/reservation/apiv1] clients when
// BIGQUERY_STORAGE_GRPC_ENDPOINT is set to host:port (no URL scheme).
// The emulator serves ReservationService on the same gRPC listener as BigQuery Storage.
// When unset, returns nil so callers can spread
// into NewClient with no effect.
func ReservationGRPCClientOptions() []option.ClientOption {
	return StorageGRPCClientOptions()
}

// ConnectionGRPCClientOptions returns options for
// [cloud.google.com/go/bigquery/connection/apiv1] clients when
// BIGQUERY_STORAGE_GRPC_ENDPOINT is set to host:port (no URL scheme).
// The emulator serves ConnectionService on the same gRPC listener as BigQuery Storage.
// When unset, returns nil so callers can spread into NewClient with no effect.
func ConnectionGRPCClientOptions() []option.ClientOption {
	return StorageGRPCClientOptions()
}

// AnalyticsHubGRPCClientOptions returns options for
// [cloud.google.com/go/bigquery/analyticshub/apiv1] clients when
// BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT is set to host:port (no URL scheme), matching the
// bigquery-emulator gRPC listener (same port as BigQuery Storage gRPC). When unset, returns nil
// so callers can spread into NewClient with no effect.
func AnalyticsHubGRPCClientOptions() []option.ClientOption {
	ep := strings.TrimSpace(os.Getenv("BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT"))
	if ep == "" {
		return nil
	}
	return []option.ClientOption{
		option.WithEndpoint(ep),
		option.WithoutAuthentication(),
		option.WithGRPCDialOption(grpc.WithTransportCredentials(insecure.NewCredentials())),
		option.WithGRPCDialOption(grpc.WithContextDialer(grpcEmulatorDialer(emulatorGRPCDialTimeout(), "BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT", ep))),
	}
}
