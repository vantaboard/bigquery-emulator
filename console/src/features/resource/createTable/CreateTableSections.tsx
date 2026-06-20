import { SectionHeading } from '@/components/ui/SectionHeading';

import { CREATE_TABLE_SOURCES } from './defaults';
import type { CreateTableFormState } from './types';

const inputClass =
    'w-full rounded-md border border-[var(--bq-border)] bg-transparent px-2 py-1.5 text-sm text-white';
const labelClass = 'mb-1 block text-xs text-[var(--bq-muted)]';

interface CreateTableSourceSectionProps {
    form: CreateTableFormState;
    onChange: (patch: Partial<CreateTableFormState>) => void;
    tableIds: string[];
}

export function CreateTableSourceSection({
    form,
    onChange,
    tableIds,
}: CreateTableSourceSectionProps) {
    return (
        <section data-testid="create-table-source">
            <SectionHeading>Create table from</SectionHeading>
            <select
                className={inputClass}
                value={form.source}
                data-testid="create-table-source-select"
                onChange={(e) => onChange({ source: e.target.value as CreateTableFormState['source'] })}
            >
                {CREATE_TABLE_SOURCES.map((opt) => (
                    <option key={opt.id} value={opt.id}>
                        {opt.label}
                    </option>
                ))}
            </select>

            {form.source === 'gcs' ? (
                <label className={`mt-3 block ${labelClass}`}>
                    GCS URI
                    <input
                        className={inputClass}
                        placeholder="gs://bucket/path/*.csv"
                        value={form.gcsUri}
                        data-testid="create-table-gcs-uri"
                        onChange={(e) => onChange({ gcsUri: e.target.value })}
                    />
                </label>
            ) : null}

            {form.source === 'upload' ? (
                <label className={`mt-3 block ${labelClass}`}>
                    File
                    <input
                        type="file"
                        className="mt-1 block w-full text-sm"
                        data-testid="create-table-upload-file"
                        onChange={(e) => onChange({ uploadFile: e.target.files?.[0] ?? null })}
                    />
                </label>
            ) : null}

            {form.source === 'drive' ? (
                <label className={`mt-3 block ${labelClass}`}>
                    Drive URI
                    <input
                        className={inputClass}
                        placeholder="https://drive.google.com/..."
                        value={form.driveUri}
                        onChange={(e) => onChange({ driveUri: e.target.value })}
                    />
                </label>
            ) : null}

            {form.source === 'bigtable' ? (
                <label className={`mt-3 block ${labelClass}`}>
                    Bigtable URI
                    <input
                        className={inputClass}
                        placeholder="https://googleapis.com/bigtable/..."
                        value={form.bigtableUri}
                        onChange={(e) => onChange({ bigtableUri: e.target.value })}
                    />
                </label>
            ) : null}

            {form.source === 's3' ? (
                <div className="mt-3 space-y-2">
                    <label className={`block ${labelClass}`}>
                        S3 URI
                        <input
                            className={inputClass}
                            placeholder="s3://bucket/path"
                            value={form.s3Uri}
                            onChange={(e) => onChange({ s3Uri: e.target.value })}
                        />
                    </label>
                    <label className={`block ${labelClass}`}>
                        Connection (optional)
                        <input
                            className={inputClass}
                            value={form.s3Connection}
                            onChange={(e) => onChange({ s3Connection: e.target.value })}
                        />
                    </label>
                </div>
            ) : null}

            {form.source === 'azure' ? (
                <div className="mt-3 space-y-2">
                    <label className={`block ${labelClass}`}>
                        Azure URI
                        <input
                            className={inputClass}
                            placeholder="azure://account/container/path"
                            value={form.azureUri}
                            onChange={(e) => onChange({ azureUri: e.target.value })}
                        />
                    </label>
                    <label className={`block ${labelClass}`}>
                        Connection (optional)
                        <input
                            className={inputClass}
                            value={form.azureConnection}
                            onChange={(e) => onChange({ azureConnection: e.target.value })}
                        />
                    </label>
                </div>
            ) : null}

            {form.source === 'existing' ? (
                <div className="mt-3 grid gap-2 sm:grid-cols-3">
                    <label className={`block ${labelClass}`}>
                        Source project
                        <input
                            className={inputClass}
                            value={form.sourceProject}
                            data-testid="create-table-source-project"
                            onChange={(e) => onChange({ sourceProject: e.target.value })}
                        />
                    </label>
                    <label className={`block ${labelClass}`}>
                        Source dataset
                        <input
                            className={inputClass}
                            value={form.sourceDataset}
                            data-testid="create-table-source-dataset"
                            onChange={(e) => onChange({ sourceDataset: e.target.value })}
                        />
                    </label>
                    <label className={`block ${labelClass}`}>
                        Source table
                        <select
                            className={inputClass}
                            value={form.sourceTable}
                            data-testid="create-table-source-table"
                            onChange={(e) => onChange({ sourceTable: e.target.value })}
                        >
                            <option value="">Select table</option>
                            {tableIds.map((id) => (
                                <option key={id} value={id}>
                                    {id}
                                </option>
                            ))}
                        </select>
                    </label>
                </div>
            ) : null}
        </section>
    );
}

interface CreateTableDestinationSectionProps {
    form: CreateTableFormState;
    onChange: (patch: Partial<CreateTableFormState>) => void;
    defaultProject: string;
    defaultDataset: string;
}

export function CreateTableDestinationSection({
    form,
    onChange,
    defaultProject,
    defaultDataset,
}: CreateTableDestinationSectionProps) {
    return (
        <section className="mt-4" data-testid="create-table-destination">
            <SectionHeading>Destination</SectionHeading>
            <div className="grid gap-2 sm:grid-cols-3">
                <label className={`block ${labelClass}`}>
                    Project
                    <input
                        className={inputClass}
                        value={form.destinationProject}
                        onChange={(e) => onChange({ destinationProject: e.target.value })}
                    />
                </label>
                <label className={`block ${labelClass}`}>
                    Dataset
                    <input
                        className={inputClass}
                        value={form.destinationDataset}
                        onChange={(e) => onChange({ destinationDataset: e.target.value })}
                    />
                </label>
                <label className={`block ${labelClass}`}>
                    Table name
                    <input
                        className={inputClass}
                        value={form.tableName}
                        placeholder="my_table"
                        data-testid="create-table-name"
                        onChange={(e) => onChange({ tableName: e.target.value })}
                    />
                </label>
            </div>
            <p className="mt-1 text-xs text-[var(--bq-muted)]">
                Table names must contain a letter or underscore, and may only include letters, numbers, and
                underscores. Defaults to {defaultProject}.{defaultDataset}.
            </p>
        </section>
    );
}
