package routines

import (
	"strconv"
	"time"
)

func nowMillis() string {
	return strconv.FormatInt(time.Now().UTC().UnixMilli(), 10)
}
