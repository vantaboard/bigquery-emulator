import { Plus, Trash2 } from 'lucide-react';

import { SCHEMA_FIELD_MODES } from '../schema/constants';
import { createEmptySchemaField } from '../schema/schemaJson';
import type { EditSchemaFieldDraft } from './editSchemaValidation';

const inputClass =
    'rounded-md border border-[var(--bq-border)] bg-transparent px-2 py-1 text-sm text-white';
const selectClass = `${inputClass} min-w-0`;
const readOnlyClass =
    'rounded-md border border-[var(--bq-border)] bg-black/20 px-2 py-1 text-sm text-[var(--bq-muted)]';

function allowedModes(field: EditSchemaFieldDraft): string[] {
    if (!field.isExisting) return [...SCHEMA_FIELD_MODES];
    if (field.originalMode === 'REQUIRED') return ['REQUIRED', 'NULLABLE'];
    return [field.originalMode];
}

interface EditSchemaFieldRowProps {
    field: EditSchemaFieldDraft;
    onChange: (field: EditSchemaFieldDraft) => void;
    onRemove?: () => void;
}

function EditSchemaFieldRow({ field, onChange, onRemove }: EditSchemaFieldRowProps) {
    const modes = allowedModes(field);

    return (
        <div
            className="flex flex-wrap items-start gap-2 rounded border border-[var(--bq-border)]/50 p-2"
            data-testid={field.name ? `edit-schema-field-${field.name}` : 'edit-schema-field-new'}
        >
            {field.isExisting ? (
                <input className={`${readOnlyClass} min-w-32 flex-1`} readOnly value={field.name} />
            ) : (
                <input
                    className={`${inputClass} min-w-32 flex-1`}
                    placeholder="Field name"
                    value={field.name}
                    data-testid="edit-schema-new-field-name"
                    onChange={(e) => onChange({ ...field, name: e.target.value })}
                />
            )}
            <input className={`${readOnlyClass} w-28`} readOnly value={field.type} />
            <select
                className={selectClass}
                value={field.mode}
                data-testid={field.name ? `edit-schema-mode-${field.name}` : 'edit-schema-mode-new'}
                onChange={(e) => onChange({ ...field, mode: e.target.value })}
            >
                {modes.map((mode) => (
                    <option key={mode} value={mode}>
                        {mode}
                    </option>
                ))}
            </select>
            <input
                className={`${inputClass} min-w-40 flex-1`}
                placeholder="Description"
                value={field.description ?? ''}
                onChange={(e) => onChange({ ...field, description: e.target.value })}
            />
            <input
                className={`${inputClass} min-w-32 flex-1`}
                placeholder="Default value"
                value={field.defaultValue ?? ''}
                onChange={(e) => onChange({ ...field, defaultValue: e.target.value })}
            />
            {!field.isExisting && onRemove ? (
                <button
                    type="button"
                    className="rounded p-1 text-[var(--bq-muted)] hover:bg-white/10 hover:text-red-400"
                    aria-label="Remove field"
                    onClick={onRemove}
                >
                    <Trash2 className="size-4" />
                </button>
            ) : null}
        </div>
    );
}

export interface EditSchemaBuilderProps {
    fields: EditSchemaFieldDraft[];
    onChange: (fields: EditSchemaFieldDraft[]) => void;
}

export function EditSchemaBuilder({ fields, onChange }: EditSchemaBuilderProps) {
    const addField = () => {
        onChange([
            ...fields,
            {
                ...createEmptySchemaField(),
                isExisting: false,
                originalMode: 'NULLABLE',
            },
        ]);
    };

    const updateField = (index: number, field: EditSchemaFieldDraft) => {
        const next = [...fields];
        next[index] = field;
        onChange(next);
    };

    const removeField = (index: number) => {
        onChange(fields.filter((_, i) => i !== index));
    };

    return (
        <div data-testid="edit-schema-builder">
            <div className="mb-2 flex justify-end">
                <button
                    type="button"
                    className="inline-flex items-center gap-1 text-sm text-blue-400 hover:underline"
                    data-testid="edit-schema-add-field"
                    onClick={addField}
                >
                    <Plus className="size-4" />
                    Add field
                </button>
            </div>
            <div className="space-y-2">
                {fields.map((field, index) => (
                    <EditSchemaFieldRow
                        key={field.id}
                        field={field}
                        onChange={(updated) => updateField(index, updated)}
                        onRemove={field.isExisting ? undefined : () => removeField(index)}
                    />
                ))}
            </div>
        </div>
    );
}
