# State of the subscription.

| Enums ||
|---|---|
| `STATE_UNSPECIFIED` | Default value. This value is unused. |
| `STATE_ACTIVE` | This subscription is active and the data is accessible. |
| `STATE_STALE` | The data referenced by this subscription is out of date and should be refreshed. This can happen when a data provider adds or removes datasets. |
| `STATE_INACTIVE` | This subscription has been cancelled or revoked and the data is no longer accessible. |