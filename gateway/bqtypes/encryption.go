package bqtypes

import "fmt"

// EncryptionConfiguration is the BigQuery REST encryptionConfiguration
// sub-object on tables and load-job destinationEncryptionConfiguration.
// The emulator stores kmsKeyName as opaque metadata only; it does not
// call Cloud KMS.
type EncryptionConfiguration struct {
	KMSKeyName string `json:"kmsKeyName,omitempty"`
}

// EmulatorCMEKKeyUSCentral returns a stable KMS crypto key resource name
// for regional CMEK samples (matches bqtestutil.EmulatorCMEKKeyUSCentral).
func EmulatorCMEKKeyUSCentral(projectID, cryptoKeyID string) string {
	return fmt.Sprintf(
		"projects/%s/locations/us-central1/keyRings/emulator/cryptoKeys/%s",
		projectID, cryptoKeyID,
	)
}
