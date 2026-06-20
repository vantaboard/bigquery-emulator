import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query';
import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { explorerQueries } from '@/features/explorer/api';
import { notifyTablesChanged } from '@/features/explorer/events';
import { newSchemaFieldId } from '@/features/resource/schema/schemaJson';

import { EditSchemaBuilder } from './EditSchemaBuilder';
import {
    editSchemaDefaultValues,
    editSchemaFieldsToTableFields,
    validateEditSchemaFields,
    type EditSchemaFieldDraft,
} from './editSchemaValidation';

export interface EditSchemaModalProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
    tableId: string;
}

function fieldsFromMetadata(
    schema: { name: string; type: string; mode: string; description?: string | null }[],
): EditSchemaFieldDraft[] {
    return schema.map((field) => ({
        id: newSchemaFieldId(),
        name: field.name,
        type: field.type,
        mode: field.mode,
        originalMode: field.mode,
        description: field.description ?? undefined,
        isExisting: true,
    }));
}

export function EditSchemaModal({ open, onClose, projectId, datasetId, tableId }: EditSchemaModalProps) {
    const queryClient = useQueryClient();
    const [fields, setFields] = useState<EditSchemaFieldDraft[]>([]);
    const [validationError, setValidationError] = useState<string | null>(null);

    const { data: metadata, isLoading } = useQuery({
        queryKey: ['explorer', 'tableSchema', projectId, datasetId, tableId],
        queryFn: () => explorerQueries.tableSchema(projectId, datasetId, tableId),
        enabled: open,
    });

    useEffect(() => {
        if (open && metadata) {
            setFields(fieldsFromMetadata(metadata.schema));
            setValidationError(null);
        }
    }, [open, metadata]);

    const saveMutation = useMutation({
        mutationFn: () =>
            explorerQueries.patchTableSchema(
                projectId,
                datasetId,
                tableId,
                editSchemaFieldsToTableFields(fields),
                { defaultValues: editSchemaDefaultValues(fields) },
            ),
        onSuccess: (updated) => {
            queryClient.setQueryData(['explorer', 'tableSchema', projectId, datasetId, tableId], updated);
            notifyTablesChanged(projectId, datasetId);
            onClose();
        },
        onError: (error: Error) => setValidationError(error.message),
    });

    const onSubmit = () => {
        const errors = validateEditSchemaFields(fields);
        if (errors.length > 0) {
            setValidationError(errors[0]?.message ?? 'Invalid schema');
            return;
        }
        setValidationError(null);
        saveMutation.mutate();
    };

    return (
        <Modal
            open={open}
            onClose={onClose}
            title="Edit schema"
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
                        data-testid="edit-schema-submit"
                        disabled={saveMutation.isPending || isLoading}
                        onClick={onSubmit}
                    >
                        {saveMutation.isPending ? 'Saving…' : 'Save schema'}
                    </button>
                </>
            }
        >
            <div data-testid="edit-schema-modal">
                {isLoading ? (
                    <p className="text-sm text-[var(--bq-muted)]">Loading schema…</p>
                ) : (
                    <section>
                        <SectionHeading>Schema fields</SectionHeading>
                        <p className="mb-2 text-xs text-[var(--bq-muted)]">
                            Existing fields: change mode (REQUIRED to NULLABLE), description, or default value.
                            Add new fields at the bottom.
                        </p>
                        <EditSchemaBuilder fields={fields} onChange={setFields} />
                    </section>
                )}

                {validationError ? (
                    <p className="mt-4 text-sm text-red-400" data-testid="edit-schema-error">
                        {validationError}
                    </p>
                ) : null}
            </div>
        </Modal>
    );
}
