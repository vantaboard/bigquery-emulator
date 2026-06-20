import CodeMirror from '@uiw/react-codemirror';
import { useMemo } from 'react';

import { buildSqlEditorExtensions } from '@/features/query/sqlEditorExtensions';
import type { SqlCatalog } from '@/features/query/sqlCatalog';
import { cn } from '@/lib/utils';

interface SqlEditorProps {
    value: string;
    onChange: (v: string) => void;
    className?: string;
    readOnly?: boolean;
    projectId?: string;
    defaultDatasetId?: string;
    useEmulatorParser?: boolean;
    sqlToolsAvailable?: boolean;
    catalog?: SqlCatalog | null;
}

export function SqlEditor({
    value,
    onChange,
    className,
    readOnly,
    projectId,
    defaultDatasetId,
    useEmulatorParser = true,
    sqlToolsAvailable = false,
    catalog = null,
}: SqlEditorProps) {
    const extensions = useMemo(
        () =>
            buildSqlEditorExtensions({
                projectId,
                defaultDatasetId,
                useEmulatorParser,
                sqlToolsAvailable,
                catalog,
            }),
        [projectId, defaultDatasetId, useEmulatorParser, sqlToolsAvailable, catalog],
    );

    return (
        <div data-testid="sql-editor">
            <CodeMirror
                value={value}
                height="220px"
                theme="dark"
                readOnly={readOnly}
                className={cn('overflow-hidden rounded-md border border-[var(--bq-border)]', className)}
                extensions={extensions}
                onChange={onChange}
                basicSetup={{
                    lineNumbers: true,
                    foldGutter: false,
                }}
            />
        </div>
    );
}
