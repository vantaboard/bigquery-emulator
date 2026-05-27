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

/**
 * `util.makeWritableStream` (multipart jobs.insert from a local file) builds
 * request options without `headers`. If Application Default Credentials are
 * missing, `makeAuthenticatedRequest`'s fallback reuses that object and
 * `teeny-request` then crashes setting multipart Content-Type on undefined.
 * Seed an empty headers object so auth merge and ADC-fallback paths are safe.
 */
const GUARD = '__goGooglesqlPatchCloudCommonMultipartHeaders';

function patch() {
  if (global[GUARD]) {
    return;
  }
  global[GUARD] = true;
  const common = require('@google-cloud/common');
  const orig = common.util.makeWritableStream;
  common.util.makeWritableStream = function patchedMakeWritableStream(
    dup,
    options,
    onComplete,
  ) {
    const mar = options.makeAuthenticatedRequest;
    const next = Object.assign({}, options, {
      makeAuthenticatedRequest(reqOpts, cb) {
        if (reqOpts.headers == null) {
          reqOpts.headers = {};
        }
        return mar.call(this, reqOpts, cb);
      },
    });
    return orig.call(this, dup, next, onComplete);
  };
}

patch();

module.exports = {patch};
