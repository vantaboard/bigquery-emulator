# MonitoredResource

- [JSON representation](https://docs.cloud.google.com/bigquery/docs/reference/analytics-hub/rest/Shared.Types/MonitoredResource#SCHEMA_REPRESENTATION)

An object representing a resource that can be used for monitoring, logging, billing, or other purposes. Examples include virtual machine instances, databases, and storage devices such as disks. The `type` field identifies a `MonitoredResourceDescriptor` object that describes the resource's schema. Information in the `labels` field identifies the actual resource and its attributes according to the schema. For example, a particular Compute Engine VM instance could be represented by the following object, because the `MonitoredResourceDescriptor` for `"gce_instance"` has labels `"project_id"`, `"instance_id"` and `"zone"`:

    { "type": "gce_instance",
      "labels": { "project_id": "my-project",
                  "instance_id": "12345678901234",
                  "zone": "us-central1-a" }}

| JSON representation |
|---|
| ``` { "type": string, "labels": { string: string, ... } } ``` |

| Fields ||
|---|---|
| `type` | `string` Required. The monitored resource type. This field must match the `type` field of a `MonitoredResourceDescriptor` object. For example, the type of a Compute Engine VM instance is `gce_instance`. Some descriptors include the service name in the type; for example, the type of a Datastream stream is `datastream.googleapis.com/Stream`. |
| `labels` | `map (key: string, value: string)` Required. Values for all of the labels listed in the associated monitored resource descriptor. For example, Compute Engine VM instances use the labels `"project_id"`, `"instance_id"`, and `"zone"`. An object containing a list of `"key": value` pairs. Example: `{ "name": "wrench", "mass": "1.3kg", "count": "3" }`. |