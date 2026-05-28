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

package datatransfer

import (
	"fmt"
	"net/url"
	"strings"
)

// authTypeOAuth is the BigQuery DataTransfer `authorizationType` value
// for connectors that authenticate via OAuth. Hoisted to a package
// const so the (otherwise repetitive) catalog entries below all
// reference the same source of truth.
const authTypeOAuth = "AUTHORIZATION_TYPE_OAUTH"

// DataSourceCatalogEntry describes a connector surfaced in the
// dataSources list/get responses. AuthorizationURLPlaceholder is
// emitted as JSON `authorizationUrl` when non-empty (an inert .invalid
// host; the emulator does not perform real OAuth or third-party
// traffic).
type DataSourceCatalogEntry struct {
	DataSourceID                   string
	DisplayName                    string
	Description                    string
	AuthorizationType              string
	DefaultDataRefreshIntervalDays int32
	AuthorizationURLPlaceholder    string
}

func buildAuthorizationPlaceholder(template, project, location, dataSourceID string) string {
	if strings.TrimSpace(template) == "" {
		return ""
	}
	if strings.Contains(template, "%") {
		return fmt.Sprintf(template, project, location, dataSourceID)
	}
	return template
}

// builtinDataSourceCatalog returns the connectors the emulator
// surfaces by default. `scheduled_query` exists as the canonical
// SQL-execution surface (the SQL runner follow-up will wire a
// Runner). `amazon_s3` satisfies CreateAmazonS3TransferIT's catalog
// probe even though no transfer is actually performed — the IT only
// asserts the create returned a name. The remaining entries cover
// the third-party `Create*Transfer.java` driver classes; each is a
// metadata-only
// stub (no transfer execution; no third-party traffic).
//
// All third-party rows use the same inert .invalid authorization-URL
// placeholder so `GET .../dataSources/{id}` returns a deterministic
// `authorizationUrl` without ever performing OAuth.
// oauthThirdPartyStubs lists the metadata-only third-party
// connectors the emulator advertises in its dataSources catalog.
// Each row maps directly onto a Create*Transfer.java IT driver class
// (Amazon S3 / Google Ad Manager / Google Ads / Campaign Manager /
// Google Play / Amazon Redshift / Teradata / YouTube Channel /
// YouTube Content Owner). The catalog only carries metadata; no
// transfer execution and no third-party traffic happens.
var oauthThirdPartyStubs = []struct {
	ID, Display, Desc string
}{
	{
		dataSourceAmazonS3,
		"Amazon S3 (emulator catalog stub)",
		"Metadata-only stub for third-party connector discovery; transfer execution and credential validation are not implemented.",
	},
	{
		dataSourceAdManager,
		"Google Ad Manager (emulator catalog stub)",
		"Metadata-only stub for the dfp_dt connector used by CreateAdManagerTransfer; transfer execution is not implemented.",
	},
	{
		dataSourceGoogleAds,
		"Google Ads (emulator catalog stub)",
		"Metadata-only stub for the adwords connector used by CreateAdsTransfer; transfer execution is not implemented.",
	},
	{
		dataSourceCampaignManager,
		"Campaign Manager (emulator catalog stub)",
		"Metadata-only stub for the dcm_dt connector used by CreateCampaignmanagerTransfer; transfer execution is not implemented.",
	},
	{
		dataSourcePlay,
		"Google Play (emulator catalog stub)",
		"Metadata-only stub for the play connector used by CreatePlayTransfer; transfer execution is not implemented.",
	},
	{
		dataSourceRedshift,
		"Amazon Redshift (emulator catalog stub)",
		"Metadata-only stub for the redshift connector used by CreateRedshiftTransfer; transfer execution and credential validation are not implemented.",
	},
	{
		dataSourceOnPremises,
		"Teradata / on-premises (emulator catalog stub)",
		"Metadata-only stub for the on_premises connector used by CreateTeradataTransfer; transfer execution and Teradata agent integration are not implemented.",
	},
	{
		dataSourceYoutubeChannel,
		"YouTube Channel (emulator catalog stub)",
		"Metadata-only stub for the youtube_channel connector used by CreateYoutubeChannelTransfer; transfer execution is not implemented.",
	},
	{
		dataSourceYoutubeContentOwner,
		"YouTube Content Owner (emulator catalog stub)",
		"Metadata-only stub for the youtube_content_owner connector used by CreateYoutubeContentOwnerTransfer; transfer execution is not implemented.",
	},
}

// oauthAuthorizationURLPlaceholder is the inert .invalid URL the
// catalog emits as `authorizationUrl` for every OAuth third-party
// stub. Lifted to a package const so the test fixtures and the
// catalog-builder share the same source of truth.
const oauthAuthorizationURLPlaceholder = "https://oauth-emulator.invalid/authorize?response_type=code&client_id=emulator-not-configured&data_source_id=%[3]s&project=%[1]s&location=%[2]s"

func builtinDataSourceCatalog() []DataSourceCatalogEntry {
	out := []DataSourceCatalogEntry{
		{
			DataSourceID:      dataSourceScheduledQuery,
			DisplayName:       "Scheduled Query (emulator)",
			Description:       "Runs BigQuery SQL on demand via startManualRuns or POST .../runs when a ScheduledQueryRunner is wired; no cron or third-party I/O.",
			AuthorizationType: "AUTHORIZATION_TYPE_GOOGLE_PLUS_AUTHORIZATION_CODE",
		},
	}
	for _, s := range oauthThirdPartyStubs {
		out = append(out, oauthStubEntry(s.ID, s.Display, s.Desc))
	}
	return out
}

// oauthStubEntry builds a metadata-only OAuth third-party catalog
// entry. The OAuth-related fields (authorization type, daily refresh,
// inert .invalid authorization URL placeholder) are the same for
// every third-party stub, so the per-row table only carries the
// fields that actually differ.
func oauthStubEntry(id, display, desc string) DataSourceCatalogEntry {
	return DataSourceCatalogEntry{
		DataSourceID:                   id,
		DisplayName:                    display,
		Description:                    desc,
		AuthorizationType:              authTypeOAuth,
		DefaultDataRefreshIntervalDays: 1,
		AuthorizationURLPlaceholder:    oauthAuthorizationURLPlaceholder,
	}
}

func (h *Handler) mergedCatalogEntries() []DataSourceCatalogEntry {
	base := builtinDataSourceCatalog()
	if h == nil || len(h.DataSourceCatalogExtras) == 0 {
		return base
	}
	byID := make(map[string]DataSourceCatalogEntry)
	order := make([]string, 0, len(base)+len(h.DataSourceCatalogExtras))
	for _, e := range base {
		id := strings.TrimSpace(e.DataSourceID)
		if id == "" {
			continue
		}
		e.DataSourceID = id
		byID[id] = e
		order = append(order, id)
	}
	for _, e := range h.DataSourceCatalogExtras {
		id := strings.TrimSpace(e.DataSourceID)
		if id == "" {
			continue
		}
		e.DataSourceID = id
		if _, exists := byID[id]; !exists {
			order = append(order, id)
		}
		byID[id] = e
	}
	out := make([]DataSourceCatalogEntry, 0, len(order))
	for _, id := range order {
		out = append(out, byID[id])
	}
	return out
}

func (h *Handler) catalogEntryByID(id string) (DataSourceCatalogEntry, bool) {
	id = strings.TrimSpace(id)
	if id == "" {
		return DataSourceCatalogEntry{}, false
	}
	for _, e := range h.mergedCatalogEntries() {
		if e.DataSourceID == id {
			return e, true
		}
	}
	return DataSourceCatalogEntry{}, false
}

func (h *Handler) dataSourceResource(project, location string, e DataSourceCatalogEntry) dataSourceResource {
	name := fmt.Sprintf("projects/%s/locations/%s/dataSources/%s", project, location, e.DataSourceID)
	r := dataSourceResource{
		Name:                           name,
		DataSourceID:                   e.DataSourceID,
		DisplayName:                    e.DisplayName,
		Description:                    e.Description,
		AuthorizationType:              e.AuthorizationType,
		DefaultDataRefreshIntervalDays: e.DefaultDataRefreshIntervalDays,
	}
	if u := buildAuthorizationPlaceholder(e.AuthorizationURLPlaceholder, project, location, e.DataSourceID); u != "" {
		// Guard: placeholders must stay on the inert host or relative;
		// never emit bare secrets.
		if parsed, err := url.Parse(u); err == nil && parsed.Scheme != "" && parsed.Host != "" {
			r.AuthorizationURL = u
		}
	}
	return r
}
