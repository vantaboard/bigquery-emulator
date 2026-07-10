import CodeMirror from '@uiw/react-codemirror';
import { useMemo, useRef } from 'react';

import {
    buildSqlEditorExtensions,
    type EditorDiagnostic,
} from '@/features/query/sqlEditorExtensions';
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
    onDiagnostics?: (diagnostics: EditorDiagnostic[]) => void;
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
    onDiagnostics,
}: SqlEditorProps) {
    const onDiagnosticsRef = useRef(onDiagnostics);
    onDiagnosticsRef.current = onDiagnostics;

    const extensions = useMemo(
        () =>
            buildSqlEditorExtensions({
                projectId,
                defaultDatasetId,
                useEmulatorParser,
                sqlToolsAvailable,
                catalog,
                onDiagnostics: (diagnostics) => onDiagnosticsRef.current?.(diagnostics),
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
