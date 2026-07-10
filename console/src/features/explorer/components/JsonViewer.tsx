import Editor, { loader } from '@monaco-editor/react';
import * as monaco from 'monaco-editor';

import { cn } from '@/lib/utils';

loader.config({ monaco });

interface JsonViewerProps {
    value: string;
    className?: string;
}

export function JsonViewer({ value, className }: JsonViewerProps) {
    return (
        <div className={cn('overflow-hidden rounded-md border border-[var(--bq-border)]', className)}>
            <Editor
                height="320px"
                language="json"
                theme="vs-dark"
                value={value}
                options={{
                    readOnly: true,
                    minimap: { enabled: false },
                    fontSize: 13,
                    lineNumbers: 'on',
                    scrollBeyondLastLine: false,
                    automaticLayout: true,
                    folding: true,
                }}
            />
        </div>
    );
}
