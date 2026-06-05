package load

import (
	"errors"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func parseAvro(_ []byte, _ *bqtypes.TableSchema, _ bool) (ParsedRows, error) {
	return ParsedRows{}, errors.New("load job sourceFormat AVRO is not yet implemented in the gateway; " +
		"use CSV, NEWLINE_DELIMITED_JSON, or PARQUET, or track thirdparty-04 follow-up",
	)
}

func parseORC(_ []byte, _ *bqtypes.TableSchema, _ bool) (ParsedRows, error) {
	return ParsedRows{}, errors.New("load job sourceFormat ORC is not yet implemented in the gateway; " +
		"use CSV, NEWLINE_DELIMITED_JSON, or PARQUET, or track thirdparty-04 follow-up",
	)
}
