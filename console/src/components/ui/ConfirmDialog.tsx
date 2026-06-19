import type { ReactNode } from 'react';

import { Modal } from './Modal';

export interface ConfirmDialogProps {
    open: boolean;
    onClose: () => void;
    onConfirm: () => void;
    title: string;
    message: ReactNode;
    confirmLabel?: string;
    cancelLabel?: string;
    confirmVariant?: 'danger' | 'primary';
    loading?: boolean;
}

export function ConfirmDialog({
    open,
    onClose,
    onConfirm,
    title,
    message,
    confirmLabel = 'Confirm',
    cancelLabel = 'Cancel',
    confirmVariant = 'danger',
    loading = false,
}: ConfirmDialogProps) {
    return (
        <Modal
            open={open}
            onClose={onClose}
            title={title}
            size="sm"
            footer={
                <>
                    <button
                        type="button"
                        className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                        disabled={loading}
                        onClick={onClose}
                    >
                        {cancelLabel}
                    </button>
                    <button
                        type="button"
                        className={
                            confirmVariant === 'danger'
                                ? 'rounded-md bg-red-600 px-3 py-1.5 text-sm font-medium hover:bg-red-500 disabled:opacity-50'
                                : 'rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium hover:bg-blue-500 disabled:opacity-50'
                        }
                        disabled={loading}
                        onClick={onConfirm}
                    >
                        {loading ? 'Working…' : confirmLabel}
                    </button>
                </>
            }
        >
            <div className="text-sm text-[var(--bq-muted)]">{message}</div>
        </Modal>
    );
}
