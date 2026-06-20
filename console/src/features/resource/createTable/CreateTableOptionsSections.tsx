import { Plus, Trash2 } from 'lucide-react';

import { SectionHeading } from '@/components/ui/SectionHeading';

import { FILE_FORMAT_OPTIONS, newKeyValueId, ROUNDING_MODES } from './defaults';
import type { CreateTableFormState, KeyValuePair } from './types';

const inputClass =
    'w-full rounded-md border border-[var(--bq-border)] bg-transparent px-2 py-1.5 text-sm text-white';
const labelClass = 'mb-1 block text-xs text-[var(--bq-muted)]';

interface KeyValueEditorProps {
    pairs: KeyValuePair[];
    onChange: (pairs: KeyValuePair[]) => void;
    testId: string;
}

function KeyValueEditor({ pairs, onChange, testId }: KeyValueEditorProps) {
    const update = (index: number, patch: Partial<KeyValuePair>) => {
        const next = [...pairs];
        next[index] = { ...next[index], ...patch };
        onChange(next);
    };

    const add = () => onChange([...pairs, { id: newKeyValueId(), key: '', value: '' }]);
    const remove = (index: number) => onChange(pairs.filter((_, i) => i !== index));

    return (
        <div data-testid={testId} className="space-y-2">
            {pairs.map((pair, index) => (
                <div key={pair.id} className="flex gap-2">
                    <input
                        className={inputClass}
                        placeholder="Key"
                        value={pair.key}
                        onChange={(e) => update(index, { key: e.target.value })}
                    />
                    <input
                        className={inputClass}
                        placeholder="Value"
                        value={pair.value}
                        onChange={(e) => update(index, { value: e.target.value })}
                    />
                    <button
                        type="button"
                        className="rounded p-1 text-[var(--bq-muted)] hover:text-red-400"
                        onClick={() => remove(index)}
                    >
                        <Trash2 className="size-4" />
                    </button>
                </div>
            ))}
            <button
                type="button"
                className="inline-flex items-center gap-1 text-sm text-blue-400 hover:underline"
                onClick={add}
            >
                <Plus className="size-4" />
                Add row
            </button>
        </div>
    );
}

interface CreateTableOptionsSectionsProps {
    form: CreateTableFormState;
    onChange: (patch: Partial<CreateTableFormState>) => void;
}

export function CreateTablePartitioningSection({ form, onChange }: CreateTableOptionsSectionsProps) {
    return (
        <section className="mt-4" data-testid="create-table-partitioning">
            <SectionHeading>Partitioning</SectionHeading>
            <select
                className={inputClass}
                value={form.partitionType}
                onChange={(e) =>
                    onChange({ partitionType: e.target.value as CreateTableFormState['partitionType'] })
                }
            >
                <option value="none">No partitioning</option>
                <option value="ingestion_time">Partition by ingestion time</option>
                <option value="field">Partition by field</option>
            </select>
            {form.partitionType === 'field' ? (
                <label className={`mt-2 block ${labelClass}`}>
                    Partition field
                    <input
                        className={inputClass}
                        value={form.partitionField}
                        onChange={(e) => onChange({ partitionField: e.target.value })}
                    />
                </label>
            ) : null}
            {form.partitionType !== 'none' ? (
                <label className={`mt-2 block ${labelClass}`}>
                    Partition expiration (days)
                    <input
                        className={inputClass}
                        type="number"
                        min={0}
                        value={form.partitionExpirationDays}
                        onChange={(e) => onChange({ partitionExpirationDays: e.target.value })}
                    />
                </label>
            ) : null}
        </section>
    );
}

export function CreateTableClusteringSection({ form, onChange }: CreateTableOptionsSectionsProps) {
    return (
        <section className="mt-4" data-testid="create-table-clustering">
            <SectionHeading>Clustering</SectionHeading>
            <label className={`block ${labelClass}`}>
                Clustering order (comma-separated fields)
                <input
                    className={inputClass}
                    placeholder="field_a, field_b"
                    value={form.clusteringFields}
                    onChange={(e) => onChange({ clusteringFields: e.target.value })}
                />
            </label>
        </section>
    );
}

export function CreateTableTagsSection({ form, onChange }: CreateTableOptionsSectionsProps) {
    return (
        <section className="mt-4" data-testid="create-table-tags">
            <SectionHeading>Tags</SectionHeading>
            <label className={`mb-2 block ${labelClass}`}>
                Tag scope
                <input
                    className={inputClass}
                    value={form.tagScope}
                    onChange={(e) => onChange({ tagScope: e.target.value })}
                />
            </label>
            <KeyValueEditor
                pairs={form.tags}
                testId="create-table-tags-editor"
                onChange={(tags) => onChange({ tags })}
            />
        </section>
    );
}

export function CreateTableAdvancedSection({ form, onChange }: CreateTableOptionsSectionsProps) {
    return (
        <section className="mt-4" data-testid="create-table-advanced">
            <SectionHeading>Advanced options</SectionHeading>
            <div className="space-y-2">
                <fieldset>
                    <legend className={`${labelClass} mb-1`}>Encryption</legend>
                    <label className="mr-4 inline-flex items-center gap-1 text-sm">
                        <input
                            type="radio"
                            checked={form.encryptionType === 'google_managed'}
                            onChange={() => onChange({ encryptionType: 'google_managed' })}
                        />
                        Google-managed key
                    </label>
                    <label className="inline-flex items-center gap-1 text-sm">
                        <input
                            type="radio"
                            checked={form.encryptionType === 'cloud_kms'}
                            onChange={() => onChange({ encryptionType: 'cloud_kms' })}
                        />
                        Cloud KMS key
                    </label>
                </fieldset>
                {form.encryptionType === 'cloud_kms' ? (
                    <input
                        className={inputClass}
                        placeholder="KMS key name"
                        value={form.kmsKeyName}
                        onChange={(e) => onChange({ kmsKeyName: e.target.value })}
                    />
                ) : null}
                <label className={`block ${labelClass}`}>
                    Default collation
                    <input
                        className={inputClass}
                        value={form.defaultCollation}
                        onChange={(e) => onChange({ defaultCollation: e.target.value })}
                    />
                </label>
                <label className={`block ${labelClass}`}>
                    Default rounding mode
                    <select
                        className={inputClass}
                        value={form.defaultRoundingMode}
                        onChange={(e) => onChange({ defaultRoundingMode: e.target.value })}
                    >
                        {ROUNDING_MODES.map((mode) => (
                            <option key={mode || 'default'} value={mode}>
                                {mode || 'Default'}
                            </option>
                        ))}
                    </select>
                </label>
                <div>
                    <p className={`${labelClass} mb-1`}>Labels</p>
                    <KeyValueEditor
                        pairs={form.labels}
                        testId="create-table-labels-editor"
                        onChange={(labels) => onChange({ labels })}
                    />
                </div>
            </div>
        </section>
    );
}

export function CreateTableUploadFormatSection({ form, onChange }: CreateTableOptionsSectionsProps) {
    const fmt = form.uploadFormat;
    const patchFormat = (patch: Partial<CreateTableFormState['uploadFormat']>) =>
        onChange({ uploadFormat: { ...fmt, ...patch } });

    return (
        <section className="mt-4" data-testid="create-table-upload-format">
            <SectionHeading>Upload format options</SectionHeading>
            <div className="grid gap-2 sm:grid-cols-2">
                <label className={`block ${labelClass}`}>
                    File format
                    <select
                        className={inputClass}
                        value={fmt.fileFormat}
                        data-testid="create-table-file-format"
                        onChange={(e) =>
                            patchFormat({ fileFormat: e.target.value as typeof fmt.fileFormat })
                        }
                    >
                        {FILE_FORMAT_OPTIONS.map((opt) => (
                            <option key={opt.value} value={opt.value}>
                                {opt.label}
                            </option>
                        ))}
                    </select>
                </label>
                <label className={`block ${labelClass}`}>
                    Write preference
                    <select
                        className={inputClass}
                        value={fmt.writeDisposition}
                        onChange={(e) =>
                            patchFormat({
                                writeDisposition: e.target.value as typeof fmt.writeDisposition,
                            })
                        }
                    >
                        <option value="WRITE_EMPTY">Write if empty</option>
                        <option value="WRITE_TRUNCATE">Write truncate</option>
                        <option value="WRITE_APPEND">Write append</option>
                    </select>
                </label>
                <label className={`block ${labelClass}`}>
                    Number of errors allowed
                    <input
                        className={inputClass}
                        type="number"
                        min={0}
                        value={fmt.maxBadRecords}
                        onChange={(e) => patchFormat({ maxBadRecords: Number(e.target.value) || 0 })}
                    />
                </label>
                {fmt.fileFormat === 'CSV' ? (
                    <>
                        <label className={`block ${labelClass}`}>
                            Field delimiter
                            <input
                                className={inputClass}
                                value={fmt.fieldDelimiter}
                                onChange={(e) => patchFormat({ fieldDelimiter: e.target.value })}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Quote character
                            <input
                                className={inputClass}
                                value={fmt.quote}
                                onChange={(e) => patchFormat({ quote: e.target.value })}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Header rows to skip
                            <input
                                className={inputClass}
                                type="number"
                                min={0}
                                value={fmt.skipLeadingRows}
                                onChange={(e) =>
                                    patchFormat({ skipLeadingRows: Number(e.target.value) || 0 })
                                }
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Null marker
                            <input
                                className={inputClass}
                                value={fmt.nullMarker}
                                onChange={(e) => patchFormat({ nullMarker: e.target.value })}
                            />
                        </label>
                        <label className={`block ${labelClass}`}>
                            Source column match
                            <input
                                className={inputClass}
                                value={fmt.sourceColumnMatch}
                                onChange={(e) => patchFormat({ sourceColumnMatch: e.target.value })}
                            />
                        </label>
                        <label className="inline-flex items-center gap-2 text-sm">
                            <input
                                type="checkbox"
                                checked={fmt.allowQuotedNewlines}
                                onChange={(e) => patchFormat({ allowQuotedNewlines: e.target.checked })}
                            />
                            Quoted newlines
                        </label>
                        <label className="inline-flex items-center gap-2 text-sm">
                            <input
                                type="checkbox"
                                checked={fmt.allowJaggedRows}
                                onChange={(e) => patchFormat({ allowJaggedRows: e.target.checked })}
                            />
                            Jagged rows
                        </label>
                    </>
                ) : null}
                <label className="inline-flex items-center gap-2 text-sm">
                    <input
                        type="checkbox"
                        checked={fmt.ignoreUnknownValues}
                        onChange={(e) => patchFormat({ ignoreUnknownValues: e.target.checked })}
                    />
                    Unknown values
                </label>
                <label className={`block ${labelClass}`}>
                    Custom timezone
                    <input
                        className={inputClass}
                        value={fmt.timeZone}
                        onChange={(e) => patchFormat({ timeZone: e.target.value })}
                    />
                </label>
                <label className={`block ${labelClass}`}>
                    Date format
                    <input
                        className={inputClass}
                        value={fmt.dateFormat}
                        onChange={(e) => patchFormat({ dateFormat: e.target.value })}
                    />
                </label>
                <label className={`block ${labelClass}`}>
                    Datetime format
                    <input
                        className={inputClass}
                        value={fmt.datetimeFormat}
                        onChange={(e) => patchFormat({ datetimeFormat: e.target.value })}
                    />
                </label>
                <label className={`block ${labelClass}`}>
                    Timestamp format
                    <input
                        className={inputClass}
                        value={fmt.timestampFormat}
                        onChange={(e) => patchFormat({ timestampFormat: e.target.value })}
                    />
                </label>
            </div>
        </section>
    );
}
