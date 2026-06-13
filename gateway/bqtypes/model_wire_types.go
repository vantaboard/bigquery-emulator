package bqtypes

// ModelReference is a stable handle to a BigQuery ML model resource.
type ModelReference struct {
	ProjectID string `json:"projectId,omitempty"`
	DatasetID string `json:"datasetId,omitempty"`
	ModelID   string `json:"modelId,omitempty"`
}

// Model mirrors the subset of the upstream Model resource REST handlers
// round-trip for metadata-only CREATE MODEL stubs (no trained weights).
type Model struct {
	ModelReference   ModelReference    `json:"modelReference"`
	ModelType        string            `json:"modelType,omitempty"`
	CreationTime     string            `json:"creationTime,omitempty"`
	LastModifiedTime string            `json:"lastModifiedTime,omitempty"`
	Labels           map[string]string `json:"labels,omitempty"`
	Description      string            `json:"description,omitempty"`
	Etag             string            `json:"etag,omitempty"`
}
