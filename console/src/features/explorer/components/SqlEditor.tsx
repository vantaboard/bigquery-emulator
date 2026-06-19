import CodeMirror from '@uiw/react-codemirror';
import { sql } from '@codemirror/lang-sql';

import { cn } from '@/lib/utils';

interface SqlEditorProps {
    value: string;
    onChange: (v: string) => void;
    className?: string;
    readOnly?: boolean;
}

export function SqlEditor({ value, onChange, className, readOnly }: SqlEditorProps) {
    return (
        <div data-testid="sql-editor">
        <CodeMirror
            value={value}
            height="220px"
            theme="dark"
            readOnly={readOnly}
            className={cn('overflow-hidden rounded-md border border-[var(--bq-border)]', className)}
            extensions={[sql()]}
            onChange={onChange}
            basicSetup={{
                lineNumbers: true,
                foldGutter: false,
            }}
        />
        </div>
    );
}
