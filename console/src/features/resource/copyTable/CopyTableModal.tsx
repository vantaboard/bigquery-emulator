import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { notifyTablesChanged } from '@/features/explorer/events';
import {
    buildCopyTableJobConfig,
    submitCopyJobAndWait,
    type TableReference,
} from '@/features/resource/jobs/copyJobs';
import { inputClass, labelClass } from '@/features/resource/shared/formStyles';
import { ReadOnlySourceSection } from '@/features/resource/shared/ReadOnlySourceSection';
import { isValidTableId } from '@/features/resource/schema/validation';

export interface CopyTableModalProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
    tableId: string;
}

export function CopyTableModal({ open, onClose, projectId, datasetId, tableId }: CopyTableModalProps) {
    const queryClient = useQueryClient();
    const [destinationProject, setDestinationProject] = useState(projectId);
    const [destinationDataset, setDestinationDataset] = useState(datasetId);
    const [destinationTable, setDestinationTable] = useState(`${tableId}_copy`);
    const [encryptionType, setEncryptionType] = useState<'google_managed' | 'cloud_kms'>('google_managed');
    const [kmsKeyName, setKmsKeyName] = useState('');
    const [showAdvanced, setShowAdvanced] = useState(false);
    const [validationError, setValidationError] = useState<string | null>(null);

    useEffect(() => {
        if (open) {
            setDestinationProject(projectId);
            setDestinationDataset(datasetId);
            setDestinationTable(`${tableId}_copy`);
            setEncryptionType('google_managed');
            setKmsKeyName('');
            setShowAdvanced(false);
            setValidationError(null);
        }
    }, [open, projectId, datasetId, tableId]);

    const copyMutation = useMutation({
        mutationFn: async () => {
            const source: TableReference = { projectId, datasetId, tableId };
            const destination: TableReference = {
                projectId: destinationProject.trim(),
                datasetId: destinationDataset.trim(),
                tableId: destinationTable.trim(),
            };
            const encryption =
                encryptionType === 'cloud_kms' && kmsKeyName.trim()
                    ? { kmsKeyName: kmsKeyName.trim() }
                    : undefined;
            await submitCopyJobAndWait(
                projectId,
                buildCopyTableJobConfig({ source, destination, encryption }),
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
        if (
            destinationProject.trim() === projectId &&
            destinationDataset.trim() === datasetId &&
            destinationTable.trim() === tableId
        ) {
            setValidationError('Destination must differ from the source table.');
            return;
        }
        if (encryptionType === 'cloud_kms' && !kmsKeyName.trim()) {
            setValidationError('Enter a Cloud KMS key name.');
            return;
        }
        setValidationError(null);
        copyMutation.mutate();
    };

    return (
        <Modal
            open={open}
            onClose={onClose}
            title="Copy table"
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
                        data-testid="copy-table-submit"
                        disabled={copyMutation.isPending}
                        onClick={onSubmit}
                    >
                        {copyMutation.isPending ? 'Copying…' : 'Copy table'}
                    </button>
                </>
            }
        >
            <div data-testid="copy-table-modal">
                <ReadOnlySourceSection
                    fields={[
                        { label: 'Project', value: projectId, testId: 'copy-table-source-project' },
                        { label: 'Dataset', value: datasetId, testId: 'copy-table-source-dataset' },
                        { label: 'Table', value: tableId, testId: 'copy-table-source-table' },
                    ]}
                />

                <section className="mt-4" data-testid="copy-table-destination">
                    <SectionHeading>Destination</SectionHeading>
                    <div className="grid gap-2 sm:grid-cols-3">
                        <label className={`block ${labelClass}`}>
                            Project
                            <input
                                className={inputClass}
                                value={destinationProject}
                                data-testid="copy-table-destination-project"
                                onChange={(e) => {
                                    setDestinationProject(e.target.value);
                                    setValidationError(null);
                                }}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Dataset
                            <input
                                className={inputClass}
                                value={destinationDataset}
                                data-testid="copy-table-destination-dataset"
                                onChange={(e) => {
                                    setDestinationDataset(e.target.value);
                                    setValidationError(null);
                                }}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Table
                            <input
                                className={inputClass}
                                value={destinationTable}
                                data-testid="copy-table-destination-table"
                                onChange={(e) => {
                                    setDestinationTable(e.target.value);
                                    setValidationError(null);
                                }}
                            />
                        </label>
                    </div>
                </section>

                <section className="mt-4">
                    <button
                        type="button"
                        className="text-sm text-blue-400 hover:underline"
                        data-testid="copy-table-advanced-toggle"
                        onClick={() => setShowAdvanced((v) => !v)}
                    >
                        {showAdvanced ? 'Hide advanced options' : 'Advanced options'}
                    </button>
                    {showAdvanced ? (
                        <div className="mt-2 space-y-2" data-testid="copy-table-advanced">
                            <fieldset>
                                <legend className={`${labelClass} mb-1`}>Encryption</legend>
                                <label className="mr-4 inline-flex items-center gap-1 text-sm">
                                    <input
                                        type="radio"
                                        checked={encryptionType === 'google_managed'}
                                        onChange={() => setEncryptionType('google_managed')}
                                    />
                                    Google-managed key
                                </label>
                                <label className="inline-flex items-center gap-1 text-sm">
                                    <input
                                        type="radio"
                                        checked={encryptionType === 'cloud_kms'}
                                        onChange={() => setEncryptionType('cloud_kms')}
                                    />
                                    Cloud KMS key
                                </label>
                            </fieldset>
                            {encryptionType === 'cloud_kms' ? (
                                <input
                                    className={inputClass}
                                    placeholder="KMS key name"
                                    value={kmsKeyName}
                                    data-testid="copy-table-kms-key"
                                    onChange={(e) => setKmsKeyName(e.target.value)}
                                />
                            ) : null}
                        </div>
                    ) : null}
                </section>

                {validationError ? (
                    <p className="mt-4 text-sm text-red-400" data-testid="copy-table-error">
                        {validationError}
                    </p>
                ) : null}
            </div>
        </Modal>
    );
}
