import { AlertCircle } from 'lucide-react';

import type { EditorDiagnostic } from '@/features/query/languageClient';

interface DiagnosticsStatusBarProps {
    diagnostics: EditorDiagnostic[];
}

export function DiagnosticsStatusBar({ diagnostics }: DiagnosticsStatusBarProps) {
    const primary =
        diagnostics.find((diagnostic) => diagnostic.severity === 'error') ?? diagnostics[0];

    if (!primary) {
        return null;
    }

    return (
        <div
            data-testid="sql-diagnostics-bar"
            className="mt-2 flex items-start gap-2 border border-[#5c2b29] bg-[#2d1514]/80 px-3 py-1.5 text-sm text-[#f28b82]"
            role="status"
            aria-live="polite"
        >
            <AlertCircle className="mt-0.5 size-4 shrink-0" aria-hidden="true" />
            <span className="min-w-0 break-words">{primary.message}</span>
        </div>
    );
}
