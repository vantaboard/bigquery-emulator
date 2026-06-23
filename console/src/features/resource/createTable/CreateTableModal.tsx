import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query';
import { useCallback, useEffect, useState } from 'react';
import { useNavigate } from 'react-router';

import { Modal } from '@/components/ui/Modal';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { explorerQueries } from '@/features/explorer/api';
import { notifyTablesChanged } from '@/features/explorer/events';
import { SchemaBuilder } from '@/features/resource/schema/SchemaBuilder';
import { isValidTableId, validateSchemaFields } from '@/features/resource/schema/validation';
import { tabRoutePath, useWorkspace } from '@/features/workspace/store';
import { resourceTabId } from '@/features/workspace/types';

import {
    CreateTableAdvancedSection,
    CreateTableClusteringSection,
    CreateTablePartitioningSection,
    CreateTableTagsSection,
    CreateTableUploadFormatSection,
} from './CreateTableOptionsSections';
import { CreateTableDestinationSection, CreateTableSourceSection } from './CreateTableSections';
import { defaultCreateTableForm } from './defaults';
import { showUploadFormatOptions, submitCreateTable } from './submitCreateTable';
import type { CreateTableFormState } from './types';

export interface CreateTableModalProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
    openTableOnSuccess?: boolean;
}

export function CreateTableModal({
    open,
    onClose,
    projectId,
    datasetId,
    openTableOnSuccess = false,
}: CreateTableModalProps) {
    const queryClient = useQueryClient();
    const navigate = useNavigate();
    const { openTableTab } = useWorkspace();
    const [form, setForm] = useState<CreateTableFormState>(() => defaultCreateTableForm(projectId, datasetId));
    const [validationError, setValidationError] = useState<string | null>(null);

    const { data: tableIds = [] } = useQuery({
        queryKey: ['explorer', 'tables', projectId, datasetId],
        queryFn: () => explorerQueries.tables(projectId, datasetId),
        enabled: open,
    });

    useEffect(() => {
        if (open) {
            setForm(defaultCreateTableForm(projectId, datasetId));
            setValidationError(null);
        }
    }, [open, projectId, datasetId]);

    const patchForm = useCallback((patch: Partial<CreateTableFormState>) => {
        setForm((prev) => ({ ...prev, ...patch }));
        setValidationError(null);
    }, []);

    const createMutation = useMutation({
        mutationFn: () => submitCreateTable(form),
        onSuccess: async () => {
            const tableId = form.tableName.trim();
            await queryClient.invalidateQueries({ queryKey: ['explorer'] });
            notifyTablesChanged(projectId, datasetId);
            onClose();
            if (openTableOnSuccess && tableId) {
                openTableTab(projectId, datasetId, tableId);
                navigate(
                    tabRoutePath({
                        type: 'table',
                        id: resourceTabId('table', projectId, datasetId, tableId),
                        projectId,
                        datasetId,
                        tableId,
                    }),
                );
            }
        },
        onError: (error: Error) => {
            setValidationError(error.message);
        },
    });

    const validate = (): string | null => {
        if (!isValidTableId(form.tableName)) {
            return 'Enter a valid table name (letters, numbers, underscores; must start with a letter or underscore).';
        }
        if (form.source === 'existing' && !form.sourceTable.trim()) {
            return 'Select a source table.';
        }
        if (form.source === 'upload' && !form.uploadFile) {
            return 'Choose a file to upload.';
        }
        if (form.source === 'gcs' && !form.gcsUri.trim()) {
            return 'Enter a GCS URI.';
        }
        if (form.source === 'drive' && !form.driveUri.trim()) {
            return 'Enter a Drive URI.';
        }
        if (form.source === 's3' && !form.s3Uri.trim()) {
            return 'Enter an S3 URI.';
        }
        if (form.source === 'azure' && !form.azureUri.trim()) {
            return 'Enter an Azure URI.';
        }
        if (form.source === 'bigtable' && !form.bigtableUri.trim()) {
            return 'Enter a Bigtable URI.';
        }
        const needsSchema =
            form.source === 'empty' ||
            form.source === 'upload' ||
            form.source === 'gcs' ||
            form.source === 'drive' ||
            form.source === 's3' ||
            form.source === 'azure' ||
            form.source === 'bigtable';
        if (needsSchema) {
            const schemaErrors = validateSchemaFields(form.schemaFields);
            if (schemaErrors.length > 0) {
                return schemaErrors[0]?.message ?? 'Invalid schema';
            }
        }
        return null;
    };

    const onSubmit = () => {
        const error = validate();
        if (error) {
            setValidationError(error);
            return;
        }
        createMutation.mutate();
    };

    const showSchema =
        form.source === 'empty' ||
        form.source === 'upload' ||
        form.source === 'gcs' ||
        form.source === 'drive' ||
        form.source === 's3' ||
        form.source === 'azure' ||
        form.source === 'bigtable';

    return (
        <Modal
            open={open}
            onClose={onClose}
            title="Create table"
            size="xl"
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
                        data-testid="create-table-submit"
                        disabled={createMutation.isPending}
                        onClick={onSubmit}
                    >
                        {createMutation.isPending ? 'Creating…' : 'Create table'}
                    </button>
                </>
            }
        >
            <div data-testid="create-table-modal">
                <CreateTableSourceSection form={form} tableIds={tableIds} onChange={patchForm} />
                <CreateTableDestinationSection
                    form={form}
                    defaultProject={projectId}
                    defaultDataset={datasetId}
                    onChange={patchForm}
                />

                {showSchema ? (
                    <section className="mt-4" data-testid="create-table-schema">
                        <SectionHeading>Schema</SectionHeading>
                        <SchemaBuilder fields={form.schemaFields} onChange={(schemaFields) => patchForm({ schemaFields })} />
                    </section>
                ) : null}

                <CreateTablePartitioningSection form={form} onChange={patchForm} />
                <CreateTableClusteringSection form={form} onChange={patchForm} />
                <CreateTableTagsSection form={form} onChange={patchForm} />
                <CreateTableAdvancedSection form={form} onChange={patchForm} />

                {showUploadFormatOptions(form.source) ? (
                    <CreateTableUploadFormatSection form={form} onChange={patchForm} />
                ) : null}

                {validationError ? (
                    <p className="mt-4 text-sm text-red-400" data-testid="create-table-error">
                        {validationError}
                    </p>
                ) : null}
            </div>
        </Modal>
    );
}
