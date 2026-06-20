import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { notifyTablesChanged } from '@/features/explorer/events';
import {
    buildSnapshotJobConfig,
    submitCopyJobAndWait,
    type TableReference,
} from '@/features/resource/jobs/copyJobs';
import { inputClass, labelClass } from '@/features/resource/shared/formStyles';
import { ReadOnlySourceSection } from '@/features/resource/shared/ReadOnlySourceSection';
import { isValidTableId } from '@/features/resource/schema/validation';

function defaultSnapshotName(tableId: string): string {
    const stamp = new Date().toISOString().replace(/[-:TZ.]/g, '').slice(0, 14);
    return `${tableId}_${stamp}`;
}

function toIsoTimestamp(localValue: string): string | undefined {
    if (!localValue.trim()) return undefined;
    const date = new Date(localValue);
    if (Number.isNaN(date.getTime())) return undefined;
    return date.toISOString();
}

function snapshotOffsetMs(snapshotTime: string): number | undefined {
    if (!snapshotTime.trim()) return undefined;
    const target = new Date(snapshotTime).getTime();
    if (Number.isNaN(target)) return undefined;
    return target - Date.now();
}

export interface CreateSnapshotModalProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
    tableId: string;
}

export function CreateSnapshotModal({
    open,
    onClose,
    projectId,
    datasetId,
    tableId,
}: CreateSnapshotModalProps) {
    const queryClient = useQueryClient();
    const [destinationProject, setDestinationProject] = useState(projectId);
    const [destinationDataset, setDestinationDataset] = useState(datasetId);
    const [destinationTable, setDestinationTable] = useState(() => defaultSnapshotName(tableId));
    const [expirationTime, setExpirationTime] = useState('');
    const [snapshotTime, setSnapshotTime] = useState('');
    const [validationError, setValidationError] = useState<string | null>(null);

    useEffect(() => {
        if (open) {
            setDestinationProject(projectId);
            setDestinationDataset(datasetId);
            setDestinationTable(defaultSnapshotName(tableId));
            setExpirationTime('');
            setSnapshotTime('');
            setValidationError(null);
        }
    }, [open, projectId, datasetId, tableId]);

    const snapshotMutation = useMutation({
        mutationFn: async () => {
            const source: TableReference = { projectId, datasetId, tableId };
            const destination: TableReference = {
                projectId: destinationProject.trim(),
                datasetId: destinationDataset.trim(),
                tableId: destinationTable.trim(),
            };
            const offset = snapshotOffsetMs(snapshotTime);
            await submitCopyJobAndWait(
                projectId,
                buildSnapshotJobConfig({
                    source,
                    destination,
                    destinationExpirationTime: toIsoTimestamp(expirationTime),
                    snapshotTimeOffsetMs: offset,
                }),
            );
        },
        onSuccess: async () => {
            await queryClient.invalidateQueries({ queryKey: ['explorer'] });
            notifyTablesChanged(destinationProject.trim(), destinationDataset.trim());
            onClose();
        },
        onError: (error: Error) => setValidationError(error.message),
    });

    const onSubmit = () => {
        if (!destinationProject.trim() || !destinationDataset.trim()) {
            setValidationError('Destination project and dataset are required.');
            return;
        }
        if (!isValidTableId(destinationTable)) {
            setValidationError(
                'Enter a valid destination table name (letters, numbers, underscores; must start with a letter or underscore).',
            );
            return;
        }
        if (snapshotTime.trim() && snapshotOffsetMs(snapshotTime) === undefined) {
            setValidationError('Enter a valid snapshot time.');
            return;
        }
        if (expirationTime.trim() && !toIsoTimestamp(expirationTime)) {
            setValidationError('Enter a valid expiration time.');
            return;
        }
        setValidationError(null);
        snapshotMutation.mutate();
    };

    return (
        <Modal
            open={open}
            onClose={onClose}
            title="Create table snapshot"
            size="lg"
            footer={
                <>
                    <button
                        type="button"
                        className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                        onClick={onClose}
                    >
                        Cancel
                    </button>
                    <button
                        type="button"
                        className="rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium hover:bg-blue-500 disabled:opacity-50"
                        data-testid="create-snapshot-submit"
                        disabled={snapshotMutation.isPending}
                        onClick={onSubmit}
                    >
                        {snapshotMutation.isPending ? 'Creating…' : 'Create snapshot'}
                    </button>
                </>
            }
        >
            <div data-testid="create-snapshot-modal">
                <ReadOnlySourceSection
                    fields={[
                        { label: 'Project', value: projectId, testId: 'snapshot-source-project' },
                        { label: 'Dataset', value: datasetId, testId: 'snapshot-source-dataset' },
                        { label: 'Table', value: tableId, testId: 'snapshot-source-table' },
                    ]}
                />

                <section className="mt-4" data-testid="snapshot-destination">
                    <SectionHeading>Destination</SectionHeading>
                    <div className="grid gap-2 sm:grid-cols-3">
                        <label className={`block ${labelClass}`}>
                            Project
                            <input
                                className={inputClass}
                                value={destinationProject}
                                data-testid="snapshot-destination-project"
                                onChange={(e) => setDestinationProject(e.target.value)}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Dataset
                            <input
                                className={inputClass}
                                value={destinationDataset}
                                data-testid="snapshot-destination-dataset"
                                onChange={(e) => setDestinationDataset(e.target.value)}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Table
                            <input
                                className={inputClass}
                                value={destinationTable}
                                data-testid="snapshot-destination-table"
                                onChange={(e) => setDestinationTable(e.target.value)}
                            />
                        </label>
                    </div>
                </section>

                <section className="mt-4 grid gap-2 sm:grid-cols-2">
                    <label className={`block ${labelClass}`}>
                        Expiration time
                        <input
                            type="datetime-local"
                            className={inputClass}
                            value={expirationTime}
                            data-testid="snapshot-expiration-time"
                            onChange={(e) => setExpirationTime(e.target.value)}
                        />
                    </label>
                    <label className={`block ${labelClass}`}>
                        Snapshot time
                        <input
                            type="datetime-local"
                            className={inputClass}
                            value={snapshotTime}
                            data-testid="snapshot-time"
                            onChange={(e) => setSnapshotTime(e.target.value)}
                        />
                    </label>
                </section>
                <p className="mt-1 text-xs text-[var(--bq-muted)]">
                    Snapshot time selects a point-in-time within the table time travel window. Leave blank for
                    the latest version.
                </p>

                {validationError ? (
                    <p className="mt-4 text-sm text-red-400" data-testid="create-snapshot-error">
                        {validationError}
                    </p>
                ) : null}
            </div>
        </Modal>
    );
}
