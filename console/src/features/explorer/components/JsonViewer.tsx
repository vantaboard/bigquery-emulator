import CodeMirror from '@uiw/react-codemirror';
import { json } from '@codemirror/lang-json';

export function JsonViewer({ value }: { value: string }) {
    return (
        <CodeMirror
            value={value}
            height="320px"
            theme="dark"
            readOnly
            className="overflow-hidden rounded-md border border-[var(--bq-border)]"
            extensions={[json()]}
            basicSetup={{ lineNumbers: true }}
        />
    );
}
