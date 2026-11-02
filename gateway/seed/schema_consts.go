package seed

const (
	bqTypeStruct   = "STRUCT"
	bqTypeRecord   = "RECORD"
	bqTypeJSON     = "JSON"
	bqModeRepeated = "REPEATED"
)

// PartitionTimeColumn is the row key that InsertRows reads a row's
// partition time from.
const PartitionTimeColumn = "_PARTITIONTIME"
