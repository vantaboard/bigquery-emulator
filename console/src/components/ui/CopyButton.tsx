import { Check, Copy } from 'lucide-react';
import { useEffect, useRef, useState } from 'react';

import { cn } from '@/lib/utils';

export interface CopyButtonProps {
    value: string;
    label?: string;
    copiedLabel?: string;
    className?: string;
    disabled?: boolean;
}

export function CopyButton({
    value,
    label = 'Copy',
    copiedLabel = 'Copied',
    className,
    disabled = false,
}: CopyButtonProps) {
    const [copied, setCopied] = useState(false);
    const timeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null);

    useEffect(() => {
        return () => {
            if (timeoutRef.current) clearTimeout(timeoutRef.current);
        };
    }, []);

    const onCopy = async () => {
        if (disabled || !value) return;

        try {
            await navigator.clipboard.writeText(value);
            setCopied(true);
            if (timeoutRef.current) clearTimeout(timeoutRef.current);
            timeoutRef.current = setTimeout(() => setCopied(false), 2000);
        } catch {
            /* ignore */
        }
    };

    return (
        <button
            type="button"
            className={cn(
                'inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5 disabled:cursor-not-allowed disabled:opacity-50',
                className,
            )}
            disabled={disabled || !value}
            onClick={() => void onCopy()}
        >
            {copied ? <Check className="size-4 text-green-400" /> : <Copy className="size-4" />}
            {copied ? copiedLabel : label}
        </button>
    );
}
