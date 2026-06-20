import { Plus, Trash2 } from 'lucide-react';
import { useCallback, useEffect, useState } from 'react';

import {
    RANGE_ELEMENT_TYPES,
    SCHEMA_FIELD_MODES,
    SCHEMA_FIELD_TYPES,
    TYPES_WITH_MAX_LENGTH,
} from './constants';
import { createEmptySchemaField, formatSchemaFieldsJson, parseSchemaFieldsJson } from './schemaJson';
import type { SchemaFieldDraft } from './types';

const inputClass =
    'rounded-md border border-[var(--bq-border)] bg-transparent px-2 py-1 text-sm text-white';
const selectClass = `${inputClass} min-w-0`;

interface SchemaFieldRowProps {
    field: SchemaFieldDraft;
    depth: number;
    onChange: (field: SchemaFieldDraft) => void;
    onRemove: () => void;
    canRemove: boolean;
    testIdPrefix?: string;
}

function SchemaFieldRow({ field, depth, onChange, onRemove, canRemove, testIdPrefix }: SchemaFieldRowProps) {
    const paddingLeft = depth * 16;
    const isRecord = field.type === 'RECORD';
    const isRange = field.type === 'RANGE';
    const showMaxLength = TYPES_WITH_MAX_LENGTH.has(field.type);

    const update = (patch: Partial<SchemaFieldDraft>) => onChange({ ...field, ...patch });

    const addNestedField = () => {
        const nested = field.fields ?? [];
        update({ fields: [...nested, createEmptySchemaField()] });
    };

    const updateNested = (index: number, nested: SchemaFieldDraft) => {
        const fields = [...(field.fields ?? [])];
        fields[index] = nested;
        update({ fields });
    };

    const removeNested = (index: number) => {
        const fields = (field.fields ?? []).filter((_, i) => i !== index);
        update({ fields });
    };

    return (
        <div className="space-y-2" style={{ paddingLeft }} data-testid={testIdPrefix}>
            <div className="flex flex-wrap items-start gap-2 rounded border border-[var(--bq-border)]/50 p-2">
                <input
                    className={`${inputClass} min-w-32 flex-1`}
                    placeholder="Field name"
                    value={field.name}
                    data-testid={field.name ? `schema-builder-field-${field.name}` : undefined}
                    onChange={(e) => update({ name: e.target.value })}
                />
                <select
                    className={selectClass}
                    value={field.type}
                    onChange={(e) => {
                        const type = e.target.value;
                        const patch: Partial<SchemaFieldDraft> = { type };
                        if (type === 'RECORD' && !field.fields?.length) {
                            patch.fields = [createEmptySchemaField()];
                        }
                        if (type !== 'RANGE') patch.rangeElementType = undefined;
                        if (!TYPES_WITH_MAX_LENGTH.has(type)) patch.maxLength = undefined;
                        update(patch);
                    }}
                >
                    {SCHEMA_FIELD_TYPES.map((t) => (
                        <option key={t} value={t}>
                            {t}
                        </option>
                    ))}
                </select>
                <select
                    className={selectClass}
                    value={field.mode}
                    onChange={(e) => update({ mode: e.target.value })}
                >
                    {SCHEMA_FIELD_MODES.map((m) => (
                        <option key={m} value={m}>
                            {m}
                        </option>
                    ))}
                </select>
                {showMaxLength ? (
                    <input
                        className={`${inputClass} w-24`}
                        type="number"
                        min={1}
                        placeholder="Max len"
                        value={field.maxLength ?? ''}
                        onChange={(e) =>
                            update({
                                maxLength: e.target.value ? Number(e.target.value) : undefined,
                            })
                        }
                    />
                ) : null}
                {isRange ? (
                    <select
                        className={selectClass}
                        value={field.rangeElementType ?? ''}
                        onChange={(e) => update({ rangeElementType: e.target.value })}
                    >
                        <option value="">Element type</option>
                        {RANGE_ELEMENT_TYPES.map((t) => (
                            <option key={t} value={t}>
                                {t}
                            </option>
                        ))}
                    </select>
                ) : null}
                <input
                    className={`${inputClass} min-w-40 flex-1`}
                    placeholder="Description"
                    value={field.description ?? ''}
                    onChange={(e) => update({ description: e.target.value })}
                />
                {canRemove ? (
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
            {isRecord ? (
                <div className="space-y-2 border-l border-[var(--bq-border)]/50 pl-2">
                    {(field.fields ?? []).map((nested, index) => (
                        <SchemaFieldRow
                            key={nested.id}
                            field={nested}
                            depth={depth + 1}
                            canRemove={(field.fields ?? []).length > 1}
                            onChange={(updated) => updateNested(index, updated)}
                            onRemove={() => removeNested(index)}
                        />
                    ))}
                    <button
                        type="button"
                        className="inline-flex items-center gap-1 text-xs text-blue-400 hover:underline"
                        onClick={addNestedField}
                    >
                        <Plus className="size-3" />
                        Add nested field
                    </button>
                </div>
            ) : null}
        </div>
    );
}

export interface SchemaBuilderProps {
    fields: SchemaFieldDraft[];
    onChange: (fields: SchemaFieldDraft[]) => void;
    testId?: string;
}

export function SchemaBuilder({ fields, onChange, testId = 'schema-builder' }: SchemaBuilderProps) {
    const [editAsText, setEditAsText] = useState(false);
    const [jsonText, setJsonText] = useState('');
    const [jsonError, setJsonError] = useState<string | null>(null);

    useEffect(() => {
        if (editAsText) {
            setJsonText(formatSchemaFieldsJson(fields));
            setJsonError(null);
        }
    }, [editAsText, fields]);

    const syncFromJson = useCallback(() => {
        try {
            const parsed = parseSchemaFieldsJson(jsonText);
            onChange(parsed);
            setJsonError(null);
        } catch (e) {
            setJsonError(e instanceof Error ? e.message : 'Invalid JSON');
        }
    }, [jsonText, onChange]);

    const addField = () => onChange([...fields, createEmptySchemaField()]);

    const updateField = (index: number, field: SchemaFieldDraft) => {
        const next = [...fields];
        next[index] = field;
        onChange(next);
    };

    const removeField = (index: number) => {
        onChange(fields.filter((_, i) => i !== index));
    };

    return (
        <div data-testid={testId}>
            <div className="mb-2 flex items-center justify-between gap-2">
                <label className="inline-flex items-center gap-2 text-sm">
                    <input
                        type="checkbox"
                        checked={editAsText}
                        data-testid="schema-builder-edit-as-text"
                        onChange={(e) => {
                            if (!e.target.checked) syncFromJson();
                            setEditAsText(e.target.checked);
                        }}
                    />
                    Edit as text
                </label>
                {!editAsText ? (
                    <button
                        type="button"
                        className="inline-flex items-center gap-1 text-sm text-blue-400 hover:underline"
                        data-testid="schema-builder-add-field"
                        onClick={addField}
                    >
                        <Plus className="size-4" />
                        Add field
                    </button>
                ) : null}
            </div>

            {editAsText ? (
                <div>
                    <textarea
                        className="h-48 w-full rounded-md border border-[var(--bq-border)] bg-black/20 p-2 font-mono text-xs"
                        value={jsonText}
                        data-testid="schema-builder-json"
                        onChange={(e) => setJsonText(e.target.value)}
                        onBlur={syncFromJson}
                    />
                    {jsonError ? <p className="mt-1 text-xs text-red-400">{jsonError}</p> : null}
                </div>
            ) : (
                <div className="space-y-2">
                    {fields.map((field, index) => (
                        <SchemaFieldRow
                            key={field.id}
                            field={field}
                            depth={0}
                            canRemove={fields.length > 1}
                            onChange={(updated) => updateField(index, updated)}
                            onRemove={() => removeField(index)}
                        />
                    ))}
                </div>
            )}
        </div>
    );
}
