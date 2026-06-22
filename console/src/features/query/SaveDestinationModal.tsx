import { useQuery } from '@tanstack/react-query';
import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';
import { explorerQueries } from '@/features/explorer/api';
import { inputClass, labelClass } from '@/features/resource/shared/formStyles';

export interface SaveDestination {
    projectId: string;
    datasetId: string;
    name: string;
}

interface SaveDestinationModalProps {
    open: boolean;
    title: string;
    nameLabel: string;
    submitLabel: string;
    defaultProjectId?: string;
    defaultDatasetId?: string;
    defaultName?: string;
    testId?: string;
    onClose: () => void;
    onSubmit: (destination: SaveDestination) => void;
}

export function SaveDestinationModal({
    open,
    title,
    nameLabel,
    submitLabel,
    defaultProjectId = '',
    defaultDatasetId = '',
    defaultName = '',
    testId = 'save-destination-modal',
    onClose,
    onSubmit,
}: SaveDestinationModalProps) {
    const [projectId, setProjectId] = useState(defaultProjectId);
    const [datasetId, setDatasetId] = useState(defaultDatasetId);
    const [name, setName] = useState(defaultName);

    const { data: projects = [] } = useQuery({
        queryKey: ['explorer', 'projects'],
        queryFn: explorerQueries.projects,
        enabled: open,
    });

    const { data: datasets = [] } = useQuery({
        queryKey: ['explorer', 'datasets', projectId],
        queryFn: () => explorerQueries.datasets(projectId),
        enabled: open && Boolean(projectId),
    });

    useEffect(() => {
        if (open) {
            setProjectId(defaultProjectId);
            setDatasetId(defaultDatasetId);
            setName(defaultName);
        }
    }, [open, defaultProjectId, defaultDatasetId, defaultName]);

    useEffect(() => {
        if (datasets.length > 0 && !datasets.includes(datasetId)) {
            setDatasetId(datasets[0]);
        }
    }, [datasets, datasetId]);

    const canSubmit = Boolean(projectId && datasetId && name.trim());

    return (
        <Modal open={open} onClose={onClose} title={title}>
            <form
                data-testid={testId}
                className="space-y-4"
                onSubmit={(e) => {
                    e.preventDefault();
                    if (!canSubmit) return;
                    onSubmit({ projectId, datasetId, name: name.trim() });
                    onClose();
                }}
            >
                <label className={`block ${labelClass}`}>
                    Project
                    <select
                        data-testid="save-destination-project"
                        className={inputClass}
                        value={projectId}
                        onChange={(e) => {
                            setProjectId(e.target.value);
                            setDatasetId('');
                        }}
                    >
                        {!projects.includes(projectId) && projectId ? (
                            <option value={projectId}>{projectId}</option>
                        ) : null}
                        {projects.map((p) => (
                            <option key={p} value={p}>
                                {p}
                            </option>
                        ))}
                    </select>
                </label>

                <label className={`block ${labelClass}`}>
                    Dataset
                    <select
                        data-testid="save-destination-dataset"
                        className={inputClass}
                        value={datasetId}
                        onChange={(e) => setDatasetId(e.target.value)}
                        disabled={datasets.length === 0}
                    >
                        {datasets.length === 0 ? (
                            <option value="">No datasets in project</option>
                        ) : null}
                        {datasets.map((d) => (
                            <option key={d} value={d}>
                                {d}
                            </option>
                        ))}
                    </select>
                </label>

                <label className={`block ${labelClass}`}>
                    {nameLabel}
                    <input
                        data-testid="save-destination-name"
                        className={inputClass}
                        value={name}
                        onChange={(e) => setName(e.target.value)}
                        autoFocus
                    />
                </label>

                <div className="flex justify-end gap-2">
                    <button
                        type="button"
                        className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                        onClick={onClose}
                    >
                        Cancel
                    </button>
                    <button
                        type="submit"
                        data-testid="save-destination-submit"
                        className="rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium hover:bg-blue-500 disabled:opacity-50"
                        disabled={!canSubmit}
                    >
                        {submitLabel}
                    </button>
                </div>
            </form>
        </Modal>
    );
}
