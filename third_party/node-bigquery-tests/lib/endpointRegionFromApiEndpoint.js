// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

'use strict';

// Keep suffix order aligned with api/apiregion/policy.go (longest first).
const regionalGoogleAPIHostSuffixes = [
  '-bigquerydatatransfer.googleapis.com',
  '-bigqueryreservation.googleapis.com',
  '-bigqueryconnection.googleapis.com',
  '-bigquerymigration.googleapis.com',
  '-bigquerydatapolicy.googleapis.com',
  '-analyticshub.googleapis.com',
  '-bigquery.googleapis.com',
];

/**
 * Returns the BigQuery API region implied by a regional Google API hostname
 * (e.g. `https://us-east4-bigquery.googleapis.com` → `us-east4`).
 * Empty string when not a recognized regional host.
 *
 * @param {string} [apiEndpoint]
 * @returns {string}
 */
function endpointRegionFromApiEndpoint(apiEndpoint) {
  if (apiEndpoint === undefined || apiEndpoint === null) {
    return '';
  }
  let host = String(apiEndpoint).trim();
  if (!host) {
    return '';
  }
  host = host.replace(/^https?:\/\//i, '');
  const slash = host.indexOf('/');
  if (slash >= 0) {
    host = host.slice(0, slash);
  }
  host = host.toLowerCase();
  if (!host) {
    return '';
  }
  for (const suf of regionalGoogleAPIHostSuffixes) {
    if (host.endsWith(suf)) {
      const prefix = host.slice(0, -suf.length);
      if (prefix) {
        return prefix;
      }
    }
  }
  return '';
}

module.exports = {endpointRegionFromApiEndpoint};
