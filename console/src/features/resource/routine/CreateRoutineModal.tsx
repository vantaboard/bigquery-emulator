import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';
import { explorerQueries } from '@/features/explorer/api';
import { notifyRoutinesChanged } from '@/features/explorer/events';
import { clearSqlCatalogCache } from '@/features/query/sqlCatalog';
import { inputClass, labelClass } from '@/features/resource/shared/formStyles';
import { isValidTableId } from '@/features/resource/schema/validation';

export type RoutineKind = 'SCALAR_FUNCTION' | 'PROCEDURE';

export interface CreateRoutineForm {
    name: string;
    kind: RoutineKind;
    argName: string;
    argType: string;
    returnType: string;
    body: string;
}

interface CreateRoutineModalProps {
    open: boolean;
    projectId: string;
    datasetId: string;
    onClose: () => void;
}

function buildRoutineDdl(projectId: string, datasetId: string, form: CreateRoutineForm): string {
    const name = form.name.trim();
    const argClause =
        form.argName.trim() && form.argType.trim()
            ? `(${form.argName.trim()} ${form.argType.trim()})`
            : '()';

    if (form.kind === 'PROCEDURE') {
        return `CREATE OR REPLACE PROCEDURE \`${projectId}\`.\`${datasetId}\`.\`${name}\`${argClause}\nBEGIN\n${form.body.trim()}\nEND`;
    }

    const returns = form.returnType.trim() || 'INT64';
    return `CREATE OR REPLACE FUNCTION \`${projectId}\`.\`${datasetId}\`.\`${name}\`${argClause} RETURNS ${returns} AS (\n${form.body.trim()}\n)`;
}

export function CreateRoutineModal({ open, projectId, datasetId, onClose }: CreateRoutineModalProps) {
    const queryClient = useQueryClient();
    const [form, setForm] = useState<CreateRoutineForm>({
        name: '',
        kind: 'SCALAR_FUNCTION',
        argName: 'x',
        argType: 'INT64',
        returnType: 'INT64',
        body: 'x + 1',
    });
    const [validationError, setValidationError] = useState<string | null>(null);

    useEffect(() => {
        if (open) {
            setForm({
                name: '',
                kind: 'SCALAR_FUNCTION',
                argName: 'x',
                argType: 'INT64',
                returnType: 'INT64',
                body: 'x + 1',
            });
            setValidationError(null);
        }
    }, [open]);

    const createMutation = useMutation({
        mutationFn: async () => {
            const ddl = buildRoutineDdl(projectId, datasetId, form);
            const result = await explorerQueries.runQuery(ddl, projectId);
            if (result.rows.length === 0 && result.total_rows === 0) {
                return;
            }
        },
        onSuccess: async () => {
            clearSqlCatalogCache(projectId);
            await queryClient.invalidateQueries({ queryKey: ['explorer'] });
            notifyRoutinesChanged(projectId, datasetId);
            onClose();
        },
        onError: (error: Error) => {
            setValidationError(error.message);
        },
    });

    const validate = (): string | null => {
        if (!isValidTableId(form.name)) {
            return 'Enter a valid routine name (letters, numbers, underscores; start with letter or underscore).';
        }
        if (!form.body.trim()) {
            return 'Enter a routine body.';
        }
        if (form.kind === 'SCALAR_FUNCTION' && !form.returnType.trim()) {
            return 'Enter a return type for the function.';
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

    return (
        <Modal
            open={open}
            onClose={onClose}
            title="Create routine"
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
                        data-testid="create-routine-submit"
                        disabled={createMutation.isPending}
                        onClick={onSubmit}
                    >
                        {createMutation.isPending ? 'Creating…' : 'Create routine'}
                    </button>
                </>
            }
        >
            <div className="space-y-4" data-testid="create-routine-modal">
                <label className={`block ${labelClass}`}>
                    Routine name
                    <input
                        className={inputClass}
                        data-testid="create-routine-name"
                        value={form.name}
                        onChange={(e) => {
                            setForm((prev) => ({ ...prev, name: e.target.value }));
                            setValidationError(null);
                        }}
                    />
                </label>

                <label className={`block ${labelClass}`}>
                    Type
                    <select
                        className={inputClass}
                        data-testid="create-routine-kind"
                        value={form.kind}
                        onChange={(e) =>
                            setForm((prev) => ({ ...prev, kind: e.target.value as RoutineKind }))
                        }
                    >
                        <option value="SCALAR_FUNCTION">Scalar function (UDF)</option>
                        <option value="PROCEDURE">Stored procedure</option>
                    </select>
                </label>

                <div className="grid gap-3 sm:grid-cols-2">
                    <label className={`block ${labelClass}`}>
                        Argument name
                        <input
                            className={inputClass}
                            data-testid="create-routine-arg-name"
                            value={form.argName}
                            onChange={(e) => setForm((prev) => ({ ...prev, argName: e.target.value }))}
                        />
                    </label>
                    <label className={`block ${labelClass}`}>
                        Argument type
                        <input
                            className={inputClass}
                            data-testid="create-routine-arg-type"
                            value={form.argType}
                            onChange={(e) => setForm((prev) => ({ ...prev, argType: e.target.value }))}
                        />
                    </label>
                </div>

                {form.kind === 'SCALAR_FUNCTION' ? (
                    <label className={`block ${labelClass}`}>
                        Return type
                        <input
                            className={inputClass}
                            data-testid="create-routine-return-type"
                            value={form.returnType}
                            onChange={(e) => setForm((prev) => ({ ...prev, returnType: e.target.value }))}
                        />
                    </label>
                ) : null}

                <label className={`block ${labelClass}`}>
                    Body
                    <textarea
                        className={`${inputClass} min-h-24 font-mono`}
                        data-testid="create-routine-body"
                        value={form.body}
                        onChange={(e) => setForm((prev) => ({ ...prev, body: e.target.value }))}
                    />
                </label>

                {validationError ? (
                    <p className="text-sm text-red-400" data-testid="create-routine-error">
                        {validationError}
                    </p>
                ) : null}
            </div>
        </Modal>
    );
}
