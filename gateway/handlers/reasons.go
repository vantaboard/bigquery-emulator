package handlers

// Shared BigQuery REST `reason` codes.
//
// These are the wire strings the BigQuery error envelope's `error.status`
// (and `error.errors[].reason`) field uses. Promoted to consts so the
// gRPC-to-HTTP mapping in errors.go / handlers.go and the regression
// tests that assert against them all reference the same source of truth
// (and so goconst stops flagging the repeated string literals).
const (
	reasonInvalid            = "invalid"
	reasonInvalidQuery       = "invalidQuery"
	reasonNotFound           = "notFound"
	reasonNotImplemented     = "notImplemented"
	reasonBackendError       = "backendError"
	reasonInternalError      = "internalError"
	reasonAccessDenied       = "accessDenied"
	reasonAuthError          = "authError"
	reasonQuotaExceeded      = "quotaExceeded"
	reasonDuplicate          = "duplicate"
	reasonFailedPrecondition = "failedPrecondition"
)

// Shared BigQuery REST resource keys used by list-response envelopes.
//
// These are the JSON-object keys the gateway emits inside
// `bigquery#datasetList` / `bigquery#tableList` / etc. Promoted from
// inline string literals so handlers all reference the same constant.
const (
	resourceKeyKind     = "kind"
	resourceKeyDatasets = "datasets"
	resourceKeyTables   = "tables"
	resourceKeyProjects = "projects"
	// resourceKeyTotalItems is the (legacy, non-paginated) item-count
	// field upstream returns on tableList / projectList responses. The
	// emulator's lists never paginate today (see DatasetList / TableList
	// comments) so totalItems mirrors the response array length.
	resourceKeyTotalItems = "totalItems"
)
