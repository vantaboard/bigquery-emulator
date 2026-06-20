import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query';
import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { explorerQueries } from '@/features/explorer/api';
import { notifyDatasetsChanged, notifyTablesChanged } from '@/features/explorer/events';
import { submitCopyDataset } from '@/features/resource/jobs/copyJobs';
import { inputClass, labelClass } from '@/features/resource/shared/formStyles';
import { ReadOnlySourceSection } from '@/features/resource/shared/ReadOnlySourceSection';
import { isValidTableId } from '@/features/resource/schema/validation';

export interface CopyDatasetModalProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
}

export function CopyDatasetModal({ open, onClose, projectId, datasetId }: CopyDatasetModalProps) {
    const queryClient = useQueryClient();
    const [destinationDataset, setDestinationDataset] = useState('');
    const [overwrite, setOverwrite] = useState(false);
    const [validationError, setValidationError] = useState<string | null>(null);

    const { data: metadata } = useQuery({
        queryKey: ['explorer', 'datasetMetadata', projectId, datasetId],
        queryFn: () => explorerQueries.datasetMetadata(projectId, datasetId),
        enabled: open,
    });

    useEffect(() => {
        if (open) {
            setDestinationDataset(`${datasetId}_copy`);
            setOverwrite(false);
            setValidationError(null);
        }
    }, [open, datasetId]);

    const copyMutation = useMutation({
        mutationFn: () =>
            submitCopyDataset({
                projectId,
                sourceProject: projectId,
                sourceDataset: datasetId,
                destProject: projectId,
                destDataset: destinationDataset.trim(),
                location: metadata?.location ?? 'US',
                overwrite,
            }),
        onSuccess: async () => {
            await queryClient.invalidateQueries({ queryKey: ['explorer'] });
            notifyDatasetsChanged(projectId);
            notifyTablesChanged(projectId, destinationDataset.trim());
            onClose();
        },
        onError: (error: Error) => setValidationError(error.message),
    });

    const onSubmit = () => {
        if (!isValidTableId(destinationDataset)) {
            setValidationError(
                'Enter a valid destination dataset name (letters, numbers, underscores; must start with a letter or underscore).',
            );
            return;
        }
        if (destinationDataset.trim() === datasetId) {
            setValidationError('Destination dataset must differ from the source dataset.');
            return;
        }
        setValidationError(null);
        copyMutation.mutate();
    };

    return (
        <Modal
            open={open}
            onClose={onClose}
            title="Copy dataset"
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
                        data-testid="copy-dataset-submit"
                        disabled={copyMutation.isPending}
                        onClick={onSubmit}
                    >
                        {copyMutation.isPending ? 'Copying…' : 'Copy dataset'}
                    </button>
                </>
            }
        >
            <div data-testid="copy-dataset-modal">
                <ReadOnlySourceSection
                    fields={[
                        { label: 'Project', value: projectId, testId: 'copy-dataset-source-project' },
                        { label: 'Dataset', value: datasetId, testId: 'copy-dataset-source-dataset' },
                        {
                            label: 'Location',
                            value: metadata?.location ?? '—',
                            testId: 'copy-dataset-source-location',
                        },
                    ]}
                />

                <section className="mt-4" data-testid="copy-dataset-destination">
                    <SectionHeading>Destination</SectionHeading>
                    <label className={`block ${labelClass}`}>
                        Dataset name
                        <input
                            className={inputClass}
                            value={destinationDataset}
                            data-testid="copy-dataset-destination-name"
                            onChange={(e) => {
                                setDestinationDataset(e.target.value);
                                setValidationError(null);
                            }}
                        />
                    </label>
                    <label className="mt-3 inline-flex items-center gap-2 text-sm">
                        <input
                            type="checkbox"
                            checked={overwrite}
                            data-testid="copy-dataset-overwrite"
                            onChange={(e) => setOverwrite(e.target.checked)}
                        />
                        Overwrite destination tables
                    </label>
                </section>

                {validationError ? (
                    <p className="mt-4 text-sm text-red-400" data-testid="copy-dataset-error">
                        {validationError}
                    </p>
                ) : null}
            </div>
        </Modal>
    );
}
