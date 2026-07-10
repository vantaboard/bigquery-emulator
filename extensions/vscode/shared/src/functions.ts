export interface FunctionDoc {
  name: string;
  signature: string;
  description: string;
}

export const GOOGLESQL_FUNCTIONS: FunctionDoc[] = [
  {
    name: 'SAFE_ADD',
    signature: 'SAFE_ADD(x, y)',
    description: 'Returns x + y. Returns NULL on overflow instead of failing.',
  },
  {
    name: 'COUNTIF',
    signature: 'COUNTIF(condition)',
    description: 'Returns the count of rows where condition is TRUE.',
  },
  {
    name: 'JSON_EXTRACT_SCALAR',
    signature: 'JSON_EXTRACT_SCALAR(json_string, json_path)',
    description: 'Extracts a scalar JSON value as a STRING.',
  },
  {
    name: 'TIMESTAMP_SUB',
    signature: 'TIMESTAMP_SUB(timestamp_expression, INTERVAL int64_expression date_part)',
    description: 'Subtracts an interval from a TIMESTAMP.',
  },
  {
    name: 'ROW_NUMBER',
    signature: 'ROW_NUMBER() OVER (window_specification)',
    description: 'Returns the sequential row number within a window partition.',
  },
];

export function lookupFunctionDoc(name: string): FunctionDoc | undefined {
  const upper = name.toUpperCase();
  return GOOGLESQL_FUNCTIONS.find((fn) => fn.name === upper);
}
