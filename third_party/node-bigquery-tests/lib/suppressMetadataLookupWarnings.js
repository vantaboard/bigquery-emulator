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
 * gcp-metadata emits MetadataLookupWarning when the GCE metadata server is
 * unreachable with an unexpected error (common on laptops / emulator runs).
 * Tests already set GOOGLE_CLOUD_PROJECT via sampleProjectEnv; the warning is noise.
 */

const GUARD = '__goGooglesqlSuppressMetadataLookupWarnings';

function isMetadataLookupWarning(warning, typeOrOptions) {
  if (typeOrOptions === 'MetadataLookupWarning') {
    return true;
  }
  if (
    typeOrOptions &&
    typeof typeOrOptions === 'object' &&
    typeOrOptions.type === 'MetadataLookupWarning'
  ) {
    return true;
  }
  if (warning && typeof warning === 'object' && warning.name === 'MetadataLookupWarning') {
    return true;
  }
  return false;
}

function install() {
  if (install.__installed) {
    return;
  }
  const original = process.emitWarning;
  if (typeof original !== 'function') {
    return;
  }
  install.__installed = true;
  process.emitWarning = function emitWarningFiltered(warning, typeOrOptions, code, ctor) {
    if (isMetadataLookupWarning(warning, typeOrOptions)) {
      return;
    }
    return original.call(process, warning, typeOrOptions, code, ctor);
  };
}

/** Preload this file in child `node` processes (sample scripts via execSync). */
function ensureNodeOptionsForChildren() {
  const modulePath = __filename;
  const needle = 'suppressMetadataLookupWarnings.js';
  const opts = process.env.NODE_OPTIONS || '';
  if (opts.includes(needle)) {
    return;
  }
  process.env.NODE_OPTIONS = opts ? `${opts} -r ${modulePath}` : `-r ${modulePath}`;
}

if (!global[GUARD]) {
  global[GUARD] = true;
  require('./patchCloudCommonMultipartHeaders');
  install();
  ensureNodeOptionsForChildren();
}

module.exports = {install, ensureNodeOptionsForChildren, isMetadataLookupWarning};
