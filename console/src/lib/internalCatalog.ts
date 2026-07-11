/** Internal catalog dataset for anonymous query result destinations. */
export const IMPLICIT_QUERY_RESULTS_DATASET = '_bqemu_query_results';

export function isUserVisibleDataset(datasetId: string): boolean {
    return datasetId !== IMPLICIT_QUERY_RESULTS_DATASET;
}
