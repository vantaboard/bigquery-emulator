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

import "strconv"

// pageWindow returns the [start, end) slice into a sorted resource
// list for the given pageSize / pageToken query knobs. pageToken is
// an integer-as-string offset (the same scheme go-googlesql's
// datatransfer handler emits and re-reads). Defaults: pageSize 100,
// max 1000, missing/invalid token resets to 0.
func pageWindow(lenNames int, pageSizeStr, pageToken string) (start, end int) {
	pageSize := 0
	if pageSizeStr != "" {
		if n, err := strconv.Atoi(pageSizeStr); err == nil && n > 0 {
			pageSize = n
		}
	}
	if pageSize <= 0 || pageSize > 1000 {
		pageSize = 100
	}
	if pageToken != "" {
		if off, err := strconv.Atoi(pageToken); err == nil && off >= 0 && off < lenNames {
			start = off
		}
	}
	end = start + pageSize
	if end > lenNames {
		end = lenNames
	}
	return start, end
}
