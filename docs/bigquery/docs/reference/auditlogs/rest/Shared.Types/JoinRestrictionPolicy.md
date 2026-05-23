# JoinRestrictionPolicy.JoinCondition

Enum for Join Restrictions policy.

| Enums ||
|---|---|
| `JOIN_CONDITION_UNSPECIFIED` | A join is neither required nor restricted on any column. Default value. |
| `JOIN_ANY` | A join is required on at least one of the specified columns. |
| `JOIN_ALL` | A join is required on all specified columns. |
| `JOIN_NOT_REQUIRED` | A join is not required, but if present it is only permitted on 'joinAllowedColumns' |
| `JOIN_BLOCKED` | Joins are blocked for all queries. |