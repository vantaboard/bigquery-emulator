import { X } from 'lucide-react';
import { useEffect, useId, useRef, type ReactNode } from 'react';
import { createPortal } from 'react-dom';

import { cn } from '@/lib/utils';

const SIZE_CLASS = {
    sm: 'max-w-md',
    md: 'max-w-lg',
    lg: 'max-w-2xl',
    xl: 'max-w-4xl',
} as const;

export type ModalSize = keyof typeof SIZE_CLASS;

export interface ModalProps {
    open: boolean;
    onClose: () => void;
    title: ReactNode;
    children: ReactNode;
    footer?: ReactNode;
    size?: ModalSize;
    className?: string;
}

const FOCUSABLE =
    'button:not([disabled]), [href], input:not([disabled]), select:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])';

export function Modal({ open, onClose, title, children, footer, size = 'md', className }: ModalProps) {
    const titleId = useId();
    const panelRef = useRef<HTMLDivElement>(null);
    const previousFocusRef = useRef<HTMLElement | null>(null);

    useEffect(() => {
        if (!open) return;

        previousFocusRef.current = document.activeElement instanceof HTMLElement ? document.activeElement : null;
        const previousOverflow = document.body.style.overflow;
        document.body.style.overflow = 'hidden';

        const panel = panelRef.current;
        const focusables = panel ? Array.from(panel.querySelectorAll<HTMLElement>(FOCUSABLE)) : [];
        focusables[0]?.focus();

        const onKeyDown = (event: KeyboardEvent) => {
            if (event.key === 'Escape') {
                event.preventDefault();
                onClose();
                return;
            }

            if (event.key !== 'Tab' || focusables.length === 0) return;

            const first = focusables[0];
            const last = focusables[focusables.length - 1];
            const active = document.activeElement;

            if (event.shiftKey && active === first) {
                event.preventDefault();
                last.focus();
            } else if (!event.shiftKey && active === last) {
                event.preventDefault();
                first.focus();
            }
        };

        document.addEventListener('keydown', onKeyDown);

        return () => {
            document.body.style.overflow = previousOverflow;
            document.removeEventListener('keydown', onKeyDown);
            previousFocusRef.current?.focus();
        };
    }, [open, onClose]);

    if (!open) return null;

    return createPortal(
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4">
            <button
                type="button"
                className="absolute inset-0 bg-black/60"
                aria-label="Close dialog"
                onClick={onClose}
            />
            <div
                ref={panelRef}
                role="dialog"
                aria-modal="true"
                aria-labelledby={titleId}
                className={cn(
                    'relative flex max-h-[min(90vh,720px)] w-full flex-col rounded-lg border border-[var(--bq-border)] bg-[var(--bq-surface)] shadow-xl',
                    SIZE_CLASS[size],
                    className,
                )}
            >
                <header className="flex shrink-0 items-center justify-between gap-3 border-b border-[var(--bq-border)] px-4 py-3">
                    <h2 id={titleId} className="text-base font-semibold">
                        {title}
                    </h2>
                    <button
                        type="button"
                        className="rounded p-1 text-[var(--bq-muted)] hover:bg-white/10 hover:text-white"
                        aria-label="Close"
                        onClick={onClose}
                    >
                        <X className="size-4" />
                    </button>
                </header>
                <div className="min-h-0 flex-1 overflow-y-auto px-4 py-3">{children}</div>
                {footer ? (
                    <footer className="flex shrink-0 items-center justify-end gap-2 border-t border-[var(--bq-border)] px-4 py-3">
                        {footer}
                    </footer>
                ) : null}
            </div>
        </div>,
        document.body,
    );
}
