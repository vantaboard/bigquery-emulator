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
 * Best-effort cleanup hooks in Mocha tests routinely call
 * `.delete({force: true}).catch(console.warn)` on datasets, tables, and
 * buckets. When the resource was already deleted by the test body (or by
 * another duplicate cleanup line), @google-cloud/common throws an ApiError
 * with `code: 404` / `reason: 'notFound'`. Those are expected during
 * cleanup; logging them as noisy stack traces makes real failures harder
 * to spot. This helper logs every other error via console.warn but
 * silently drops 404 NotFound errors.
 */

function isNotFoundError(err) {
  if (!err || typeof err !== 'object') {
    return false;
  }
  if (err.code === 404) {
    return true;
  }
  const errors = Array.isArray(err.errors) ? err.errors : null;
  if (errors && errors.length > 0 && errors[0] && errors[0].reason === 'notFound') {
    return true;
  }
  if (typeof err.message === 'string' && /^Not found:/i.test(err.message)) {
    return true;
  }
  return false;
}

function warnIgnoringNotFound(err) {
  if (isNotFoundError(err)) {
    return;
  }
  console.warn(err);
}

module.exports = {warnIgnoringNotFound, isNotFoundError};
