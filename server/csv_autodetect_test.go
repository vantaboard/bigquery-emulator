package server

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/types"
)

func TestIsNullValue(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		{"", true},
		{"  ", true},
		{"null", true},
		{"NULL", true},
		{"Null", true},
		{" null ", true},
		{"0", false},
		{"false", false},
		{"none", false},
		{"hello", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isNullValue(tc.input)
			if result != tc.expected {
				t.Errorf("isNullValue(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsBool(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		{"true", true},
		{"false", true},
		{"TRUE", true},
		{"FALSE", true},
		{"True", true},
		{"t", true},
		{"f", true},
		{"T", true},
		{"F", true},
		{"yes", true},
		{"no", true},
		{"YES", true},
		{"NO", true},
		{"y", true},
		{"n", true},
		{"Y", true},
		{"N", true},
		// 1/0 are NOT auto-detected as boolean (BigQuery behavior)
		{"1", false},
		{"0", false},
		{"", false},
		{"maybe", false},
		{"2", false},
		{"truee", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isBool(tc.input)
			if result != tc.expected {
				t.Errorf("isBool(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsInteger(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		{"0", true},
		{"1", true},
		{"-1", true},
		{"123456789", true},
		{"-123456789", true},
		{"  42  ", true},
		{"1.5", false},
		{"1e5", false},
		{"abc", false},
		{"", false},
		{"12.0", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isInteger(tc.input)
			if result != tc.expected {
				t.Errorf("isInteger(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsFloat(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		{"0", true},
		{"1", true},
		{"1.5", true},
		{"-1.5", true},
		{"3.14159", true},
		{"1e5", true},
		{"1.5e-10", true},
		{"  3.14  ", true},
		{"abc", false},
		{"", false},
		{"1.2.3", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isFloat(tc.input)
			if result != tc.expected {
				t.Errorf("isFloat(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsDate(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		// Valid ISO dates
		{"2024-01-15", true},
		{"2024-12-31", true},
		{"2000-01-01", true},
		{"  2024-01-15  ", true}, // Whitespace trimmed
		// Invalid formats (UK/US not supported by BigQuery)
		{"15/01/2024", false},
		{"01/15/2024", false},
		{"15-01-2024", false},
		{"01-15-2024", false},
		// Invalid values
		{"not-a-date", false},
		{"", false},
		{"2024/01/15", false}, // Slash separator not valid for DATE
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isDate(tc.input)
			if result != tc.expected {
				t.Errorf("isDate(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsTime(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		// Valid times
		{"10:30:00", true},
		{"10:30:00.123456", true},
		{"10:30:00.123", true},
		{"23:59:59", true},
		{"00:00:00", true},
		{"  14:30:00  ", true}, // Whitespace trimmed
		// Invalid times
		{"10:30", false},     // Missing seconds
		{"25:00:00", false},  // Invalid hour
		{"10:60:00", false},  // Invalid minute
		{"10:30:00Z", false}, // Has timezone - not TIME
		{"hello", false},
		{"", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isTime(tc.input)
			if result != tc.expected {
				t.Errorf("isTime(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsDatetime(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		// Valid datetimes
		{"2024-01-15 10:30:00", true},
		{"2024-01-15 10:30:00.123456", true},
		{"2024-01-15T10:30:00", true},
		{"2024-01-15T10:30:00.123456", true},
		{"  2024-01-15 10:30:00  ", true}, // Whitespace trimmed
		// Invalid - has timezone (should be TIMESTAMP)
		{"2024-01-15 10:30:00Z", false},
		{"2024-01-15 10:30:00 UTC", false},
		{"2024-01-15 10:30:00-05:00", false},
		{"2024-01-15T10:30:00Z", false},
		// Invalid - not a datetime
		{"2024-01-15", false}, // No time - DATE
		{"10:30:00", false},   // No date - TIME
		{"hello", false},
		{"", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isDatetime(tc.input)
			if result != tc.expected {
				t.Errorf("isDatetime(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestIsTimestamp(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
	}{
		// With Z suffix
		{"2024-01-15T10:30:00Z", true},
		{"2024-01-15T10:30:00.123456Z", true},
		{"2024-01-15 10:30:00Z", true},
		// With UTC
		{"2024-01-15 10:30:00 UTC", true},
		// With offset
		{"2024-01-15T10:30:00-05:00", true},
		{"2024-01-15 10:30:00 -05:00", true},
		{"2024-01-15 10:30:00.220 -05:00", true},
		// Slash date format
		{"2018/08/19 12:11", true},
		{"2018/08/19 12:11:35", true},
		{"2018/08/19 12:11:35.22", true},
		// Unix epoch is NOT auto-detected per BigQuery docs
		{"1534680695", false},
		{"1.534680695e12", false},
		// Invalid cases
		{"2024-01-15 10:30:00", false}, // No timezone - DATETIME
		{"2024-01-15", false},          // No time - DATE
		{"hello", false},
		{"", false},
		{"123", false},
		{"0", false},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result := isTimestamp(tc.input)
			if result != tc.expected {
				t.Errorf("isTimestamp(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestInferFieldType(t *testing.T) {
	testCases := []struct {
		name         string
		rows         [][]string
		colIndex     int
		expectedType string
	}{
		{
			name:         "bool_true_false",
			rows:         [][]string{{"true"}, {"false"}, {"true"}},
			colIndex:     0,
			expectedType: "BOOL",
		},
		{
			name:         "bool_yes_no",
			rows:         [][]string{{"yes"}, {"no"}, {"YES"}},
			colIndex:     0,
			expectedType: "BOOL",
		},
		{
			name:         "bool_t_f",
			rows:         [][]string{{"t"}, {"f"}, {"T"}},
			colIndex:     0,
			expectedType: "BOOL",
		},
		{
			name:         "integer",
			rows:         [][]string{{"1"}, {"42"}, {"-100"}},
			colIndex:     0,
			expectedType: "INTEGER",
		},
		{
			name:         "integer_zero_one_not_bool",
			rows:         [][]string{{"0"}, {"1"}, {"0"}},
			colIndex:     0,
			expectedType: "INTEGER", // Changed: 0/1 are not auto-detected as BOOL
		},
		{
			name:         "float",
			rows:         [][]string{{"1.5"}, {"3.14"}, {"-0.5"}},
			colIndex:     0,
			expectedType: "FLOAT",
		},
		{
			name:         "date_iso",
			rows:         [][]string{{"2024-01-15"}, {"2024-12-31"}},
			colIndex:     0,
			expectedType: "DATE",
		},
		{
			name:         "date_uk_now_string",
			rows:         [][]string{{"15/01/2024"}, {"31/12/2024"}},
			colIndex:     0,
			expectedType: "STRING", // Changed: UK format not supported
		},
		{
			name:         "time",
			rows:         [][]string{{"10:30:00"}, {"14:45:30.123456"}},
			colIndex:     0,
			expectedType: "TIME",
		},
		{
			name:         "datetime",
			rows:         [][]string{{"2024-01-15 10:30:00"}, {"2024-12-31 23:59:59"}},
			colIndex:     0,
			expectedType: "DATETIME",
		},
		{
			name:         "datetime_with_t",
			rows:         [][]string{{"2024-01-15T10:30:00"}, {"2024-12-31T23:59:59"}},
			colIndex:     0,
			expectedType: "DATETIME",
		},
		{
			name:         "timestamp_with_z",
			rows:         [][]string{{"2024-01-15T10:30:00Z"}, {"2024-12-31T23:59:59Z"}},
			colIndex:     0,
			expectedType: "TIMESTAMP",
		},
		{
			name:         "timestamp_with_offset",
			rows:         [][]string{{"2024-01-15 10:30:00-05:00"}, {"2024-12-31 23:59:59+00:00"}},
			colIndex:     0,
			expectedType: "TIMESTAMP",
		},
		{
			name:         "timestamp_slash_date",
			rows:         [][]string{{"2018/08/19 12:11:35"}, {"2018/08/20 14:30:00"}},
			colIndex:     0,
			expectedType: "TIMESTAMP",
		},
		{
			name:         "unix_epoch_is_integer",
			rows:         [][]string{{"1534680695"}, {"1534680700"}},
			colIndex:     0,
			expectedType: "INTEGER", // Unix timestamps are NOT auto-detected per BigQuery docs
		},
		{
			name:         "string",
			rows:         [][]string{{"hello"}, {"world"}},
			colIndex:     0,
			expectedType: "STRING",
		},
		{
			name:         "mixed_to_string",
			rows:         [][]string{{"1"}, {"hello"}},
			colIndex:     0,
			expectedType: "STRING",
		},
		{
			name:         "all_null",
			rows:         [][]string{{""}, {"null"}, {"NULL"}},
			colIndex:     0,
			expectedType: "STRING",
		},
		{
			name:         "integer_with_nulls",
			rows:         [][]string{{"1"}, {""}, {"3"}},
			colIndex:     0,
			expectedType: "INTEGER",
		},
		{
			name:         "second_column",
			rows:         [][]string{{"a", "1"}, {"b", "2"}, {"c", "3"}},
			colIndex:     1,
			expectedType: "INTEGER",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := inferFieldType(tc.rows, tc.colIndex)
			if result != tc.expectedType {
				t.Errorf("inferFieldType(%v, %d) = %q, want %q", tc.rows, tc.colIndex, result, tc.expectedType)
			}
		})
	}
}

func TestSelectSampleRows(t *testing.T) {
	testCases := []struct {
		name        string
		rows        [][]string
		maxSamples  int
		expectedLen int
	}{
		{
			name:        "fewer_than_max",
			rows:        [][]string{{"a"}, {"b"}, {"c"}},
			maxSamples:  10,
			expectedLen: 3,
		},
		{
			name:        "exactly_max",
			rows:        [][]string{{"a"}, {"b"}, {"c"}, {"d"}, {"e"}},
			maxSamples:  5,
			expectedLen: 5,
		},
		{
			name:        "more_than_max",
			rows:        [][]string{{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}, {"j"}, {"k"}, {"l"}},
			maxSamples:  5,
			expectedLen: 5,
		},
		{
			name:        "empty",
			rows:        [][]string{},
			maxSamples:  10,
			expectedLen: 0,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := selectSampleRows(tc.rows, tc.maxSamples)
			if len(result) != tc.expectedLen {
				t.Errorf("selectSampleRows() returned %d rows, want %d", len(result), tc.expectedLen)
			}
			// Verify first and last rows are included if we have more than maxSamples
			if len(tc.rows) > tc.maxSamples && len(result) > 0 {
				if result[0][0] != tc.rows[0][0] {
					t.Errorf("first row not included in sample")
				}
				if result[len(result)-1][0] != tc.rows[len(tc.rows)-1][0] {
					t.Errorf("last row not included in sample")
				}
			}
		})
	}
}

func TestConvertCSVValue(t *testing.T) {
	testCases := []struct {
		name     string
		value    string
		colType  types.Type
		expected interface{}
		wantErr  bool
	}{
		{
			name:     "integer",
			value:    "42",
			colType:  types.INT64,
			expected: int64(42),
			wantErr:  false,
		},
		{
			name:     "negative_integer",
			value:    "-123",
			colType:  types.INT64,
			expected: int64(-123),
			wantErr:  false,
		},
		{
			name:     "float",
			value:    "3.14",
			colType:  types.FLOAT64,
			expected: 3.14,
			wantErr:  false,
		},
		{
			name:     "bool_true",
			value:    "true",
			colType:  types.BOOL,
			expected: true,
			wantErr:  false,
		},
		{
			name:     "bool_false",
			value:    "false",
			colType:  types.BOOL,
			expected: false,
			wantErr:  false,
		},
		{
			name:     "bool_yes",
			value:    "yes",
			colType:  types.BOOL,
			expected: true,
			wantErr:  false,
		},
		{
			name:     "bool_no",
			value:    "no",
			colType:  types.BOOL,
			expected: false,
			wantErr:  false,
		},
		{
			name:     "bool_y",
			value:    "Y",
			colType:  types.BOOL,
			expected: true,
			wantErr:  false,
		},
		{
			name:     "bool_n",
			value:    "N",
			colType:  types.BOOL,
			expected: false,
			wantErr:  false,
		},
		{
			name:     "date_iso",
			value:    "2024-01-15",
			colType:  types.DATE,
			expected: "2024-01-15",
			wantErr:  false,
		},
		{
			name:     "date_uk_now_error",
			value:    "15/01/2024",
			colType:  types.DATE,
			expected: nil,
			wantErr:  true, // UK format not supported
		},
		{
			name:     "date_us_now_error",
			value:    "01/15/2024",
			colType:  types.DATE,
			expected: nil,
			wantErr:  true, // US format not supported
		},
		{
			name:     "time",
			value:    "10:30:00",
			colType:  types.TIME,
			expected: "10:30:00.000000",
			wantErr:  false,
		},
		{
			name:     "time_with_microseconds",
			value:    "10:30:00.123456",
			colType:  types.TIME,
			expected: "10:30:00.123456",
			wantErr:  false,
		},
		{
			name:     "datetime",
			value:    "2024-01-15 10:30:00",
			colType:  types.DATETIME,
			expected: "2024-01-15 10:30:00.000000",
			wantErr:  false,
		},
		{
			name:     "datetime_with_t",
			value:    "2024-01-15T10:30:00",
			colType:  types.DATETIME,
			expected: "2024-01-15 10:30:00.000000",
			wantErr:  false,
		},
		{
			name:     "timestamp_z",
			value:    "2024-01-15T10:30:00Z",
			colType:  types.TIMESTAMP,
			expected: "2024-01-15T10:30:00Z",
			wantErr:  false,
		},
		{
			name:     "timestamp_offset",
			value:    "2024-01-15 10:30:00-05:00",
			colType:  types.TIMESTAMP,
			expected: "2024-01-15T15:30:00Z", // Converted to UTC
			wantErr:  false,
		},
		{
			name:     "string",
			value:    "hello world",
			colType:  types.STRING,
			expected: "hello world",
			wantErr:  false,
		},
		{
			name:     "null_empty",
			value:    "",
			colType:  types.INT64,
			expected: nil,
			wantErr:  false,
		},
		{
			name:     "null_literal",
			value:    "null",
			colType:  types.STRING,
			expected: nil,
			wantErr:  false,
		},
		{
			name:     "null_literal_uppercase",
			value:    "NULL",
			colType:  types.INT64,
			expected: nil,
			wantErr:  false,
		},
		{
			name:     "invalid_integer",
			value:    "not_a_number",
			colType:  types.INT64,
			expected: nil,
			wantErr:  true,
		},
		{
			name:     "invalid_bool",
			value:    "maybe",
			colType:  types.BOOL,
			expected: nil,
			wantErr:  true,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result, err := convertCSVValue(tc.value, tc.colType)
			if tc.wantErr {
				if err == nil {
					t.Errorf("convertCSVValue(%q, %v) expected error but got none", tc.value, tc.colType)
				}
				return
			}
			if err != nil {
				t.Errorf("convertCSVValue(%q, %v) unexpected error: %v", tc.value, tc.colType, err)
				return
			}
			if result != tc.expected {
				t.Errorf("convertCSVValue(%q, %v) = %v (%T), want %v (%T)", tc.value, tc.colType, result, result, tc.expected, tc.expected)
			}
		})
	}
}

func TestParseBoolValue(t *testing.T) {
	testCases := []struct {
		input    string
		expected bool
		wantErr  bool
	}{
		{"true", true, false},
		{"TRUE", true, false},
		{"True", true, false},
		{"t", true, false},
		{"T", true, false},
		{"false", false, false},
		{"FALSE", false, false},
		{"False", false, false},
		{"f", false, false},
		{"F", false, false},
		{"yes", true, false},
		{"YES", true, false},
		{"no", false, false},
		{"NO", false, false},
		{"y", true, false},
		{"Y", true, false},
		{"n", false, false},
		{"N", false, false},
		// 1/0 are still parsed (for explicit schemas) but not auto-detected
		{"1", true, false},
		{"0", false, false},
		{"maybe", false, true},
		{"", false, true},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, err := parseBoolValue(tc.input)
			if tc.wantErr {
				if err == nil {
					t.Errorf("parseBoolValue(%q) expected error but got none", tc.input)
				}
				return
			}
			if err != nil {
				t.Errorf("parseBoolValue(%q) unexpected error: %v", tc.input, err)
				return
			}
			if result != tc.expected {
				t.Errorf("parseBoolValue(%q) = %v, want %v", tc.input, result, tc.expected)
			}
		})
	}
}

func TestParseAndConvertDate(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
		wantErr  bool
	}{
		{"2024-01-15", "2024-01-15", false},
		// UK/US formats not supported
		{"15/01/2024", "", true},
		{"01/15/2024", "", true},
		{"15-01-2024", "", true},
		{"01-15-2024", "", true},
		{"invalid", "", true},
		{"2024/01/15", "", true}, // Slashes are not supported
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, err := parseAndConvertDate(tc.input)
			if tc.wantErr {
				if err == nil {
					t.Errorf("parseAndConvertDate(%q) expected error but got none", tc.input)
				}
				return
			}
			if err != nil {
				t.Errorf("parseAndConvertDate(%q) unexpected error: %v", tc.input, err)
				return
			}
			if result != tc.expected {
				t.Errorf("parseAndConvertDate(%q) = %q, want %q", tc.input, result, tc.expected)
			}
		})
	}
}

func TestParseTimeValue(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
		wantErr  bool
	}{
		{"10:30:00", "10:30:00.000000", false},
		{"10:30:00.123456", "10:30:00.123456", false},
		{"10:30:00.123", "10:30:00.123000", false},
		{"23:59:59", "23:59:59.000000", false},
		{"00:00:00", "00:00:00.000000", false},
		{"invalid", "", true},
		{"10:30", "", true}, // Missing seconds
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, err := parseTimeValue(tc.input)
			if tc.wantErr {
				if err == nil {
					t.Errorf("parseTimeValue(%q) expected error but got none", tc.input)
				}
				return
			}
			if err != nil {
				t.Errorf("parseTimeValue(%q) unexpected error: %v", tc.input, err)
				return
			}
			if result != tc.expected {
				t.Errorf("parseTimeValue(%q) = %q, want %q", tc.input, result, tc.expected)
			}
		})
	}
}

func TestParseDatetimeValue(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
		wantErr  bool
	}{
		{"2024-01-15 10:30:00", "2024-01-15 10:30:00.000000", false},
		{"2024-01-15 10:30:00.123456", "2024-01-15 10:30:00.123456", false},
		{"2024-01-15T10:30:00", "2024-01-15 10:30:00.000000", false},
		{"2024-01-15T10:30:00.123456", "2024-01-15 10:30:00.123456", false},
		{"invalid", "", true},
		{"2024-01-15", "", true}, // No time
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, err := parseDatetimeValue(tc.input)
			if tc.wantErr {
				if err == nil {
					t.Errorf("parseDatetimeValue(%q) expected error but got none", tc.input)
				}
				return
			}
			if err != nil {
				t.Errorf("parseDatetimeValue(%q) unexpected error: %v", tc.input, err)
				return
			}
			if result != tc.expected {
				t.Errorf("parseDatetimeValue(%q) = %q, want %q", tc.input, result, tc.expected)
			}
		})
	}
}

func TestParseTimestampValue(t *testing.T) {
	testCases := []struct {
		input    string
		expected string
		wantErr  bool
	}{
		{"2024-01-15T10:30:00Z", "2024-01-15T10:30:00Z", false},
		{"2024-01-15 10:30:00Z", "2024-01-15T10:30:00Z", false},
		{"2024-01-15T10:30:00-05:00", "2024-01-15T15:30:00Z", false}, // Converted to UTC
		{"2024-01-15 10:30:00 UTC", "2024-01-15T10:30:00Z", false},
		{"2018/08/19 12:11:35", "2018-08-19T12:11:35Z", false}, // Slash date
		// Unix timestamps CAN be parsed (for explicit schema) even though not auto-detected
		{"1534680695", "2018-08-19T12:11:35Z", false},
		{"invalid", "", true},
	}

	for _, tc := range testCases {
		t.Run(tc.input, func(t *testing.T) {
			result, err := parseTimestampValue(tc.input)
			if tc.wantErr {
				if err == nil {
					t.Errorf("parseTimestampValue(%q) expected error but got none", tc.input)
				}
				return
			}
			if err != nil {
				t.Errorf("parseTimestampValue(%q) unexpected error: %v", tc.input, err)
				return
			}
			if result != tc.expected {
				t.Errorf("parseTimestampValue(%q) = %q, want %q", tc.input, result, tc.expected)
			}
		})
	}
}

func TestDetectSchema(t *testing.T) {
	h := &uploadContentHandler{}

	testCases := []struct {
		name            string
		records         [][]string
		skipLeadingRows int64
		expectedFields  map[string]string // column name -> type
		wantErr         bool
	}{
		{
			name: "basic_types",
			records: [][]string{
				{"name", "age", "score", "active"},
				{"Alice", "30", "95.5", "true"},
				{"Bob", "25", "88.0", "false"},
			},
			skipLeadingRows: 0,
			expectedFields: map[string]string{
				"name":   "STRING",
				"age":    "INTEGER",
				"score":  "FLOAT",
				"active": "BOOL",
			},
			wantErr: false,
		},
		{
			name: "with_dates",
			records: [][]string{
				{"event", "date"},
				{"Birthday", "2024-01-15"},
				{"Anniversary", "2024-12-31"},
			},
			skipLeadingRows: 0,
			expectedFields: map[string]string{
				"event": "STRING",
				"date":  "DATE",
			},
			wantErr: false,
		},
		{
			name: "with_nulls",
			records: [][]string{
				{"id", "value"},
				{"1", "100"},
				{"2", ""},
				{"3", "null"},
				{"4", "200"},
			},
			skipLeadingRows: 0,
			expectedFields: map[string]string{
				"id":    "INTEGER",
				"value": "INTEGER",
			},
			wantErr: false,
		},
		{
			name: "all_null_column",
			records: [][]string{
				{"id", "empty"},
				{"1", ""},
				{"2", "null"},
			},
			skipLeadingRows: 0,
			expectedFields: map[string]string{
				"id":    "INTEGER",
				"empty": "STRING",
			},
			wantErr: false,
		},
		{
			name: "header_only",
			records: [][]string{
				{"col1", "col2"},
			},
			skipLeadingRows: 0,
			expectedFields: map[string]string{
				"col1": "STRING",
				"col2": "STRING",
			},
			wantErr: false,
		},
		{
			name:            "empty_csv",
			records:         [][]string{},
			skipLeadingRows: 0,
			expectedFields:  nil,
			wantErr:         true,
		},
		{
			// skipLeadingRows=1 means only skip the header row, which is already
			// extracted, so all data rows should be used for type inference
			name: "skip_leading_rows_1",
			records: [][]string{
				{"name", "value"},
				{"Alice", "100"},
				{"Bob", "200"},
				{"Charlie", "300"},
			},
			skipLeadingRows: 1,
			expectedFields: map[string]string{
				"name":  "STRING",
				"value": "INTEGER",
			},
			wantErr: false,
		},
		{
			// skipLeadingRows=2 means skip header + 1 data row, so type inference
			// should only see rows after the first data row
			name: "skip_leading_rows_2",
			records: [][]string{
				{"name", "value"},
				{"SKIP_ME", "not_a_number"}, // This row should be skipped
				{"Bob", "200"},
				{"Charlie", "300"},
			},
			skipLeadingRows: 2,
			expectedFields: map[string]string{
				"name":  "STRING",
				"value": "INTEGER", // Would be STRING if first data row wasn't skipped
			},
			wantErr: false,
		},
		{
			// skipLeadingRows=3 means skip header + 2 data rows
			name: "skip_leading_rows_3",
			records: [][]string{
				{"id", "score"},
				{"skip1", "not_number"},
				{"skip2", "also_not_number"},
				{"1", "95.5"},
				{"2", "88.0"},
			},
			skipLeadingRows: 3,
			expectedFields: map[string]string{
				"id":    "INTEGER",
				"score": "FLOAT",
			},
			wantErr: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			schema, err := h.detectSchema(tc.records, tc.skipLeadingRows)
			if tc.wantErr {
				if err == nil {
					t.Errorf("detectSchema() expected error but got none")
				}
				return
			}
			if err != nil {
				t.Errorf("detectSchema() unexpected error: %v", err)
				return
			}

			if len(schema.Fields) != len(tc.expectedFields) {
				t.Errorf("detectSchema() returned %d fields, want %d", len(schema.Fields), len(tc.expectedFields))
				return
			}

			for _, field := range schema.Fields {
				expectedType, ok := tc.expectedFields[field.Name]
				if !ok {
					t.Errorf("unexpected field %q in schema", field.Name)
					continue
				}
				if field.Type != expectedType {
					t.Errorf("field %q has type %q, want %q", field.Name, field.Type, expectedType)
				}
				if field.Mode != "NULLABLE" {
					t.Errorf("field %q has mode %q, want NULLABLE", field.Name, field.Mode)
				}
			}
		})
	}
}
