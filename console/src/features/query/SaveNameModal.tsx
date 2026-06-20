import { useEffect, useState } from 'react';

import { Modal } from '@/components/ui/Modal';

interface SaveNameModalProps {
    open: boolean;
    title: string;
    label: string;
    defaultValue?: string;
    submitLabel: string;
    onClose: () => void;
    onSubmit: (name: string) => void;
    testId?: string;
}

export function SaveNameModal({
    open,
    title,
    label,
    defaultValue = '',
    submitLabel,
    onClose,
    onSubmit,
    testId = 'save-name-modal',
}: SaveNameModalProps) {
    const [name, setName] = useState(defaultValue);

    useEffect(() => {
        if (open) setName(defaultValue);
    }, [open, defaultValue]);

    return (
        <Modal open={open} onClose={onClose} title={title}>
            <form
                data-testid={testId}
                className="space-y-4"
                onSubmit={(e) => {
                    e.preventDefault();
                    const trimmed = name.trim();
                    if (!trimmed) return;
                    onSubmit(trimmed);
                    onClose();
                }}
            >
                <label className="block text-sm">
                    <span className="mb-1 block text-white/70">{label}</span>
                    <input
                        data-testid="save-name-input"
                        className="w-full rounded-md border border-[var(--bq-border)] bg-transparent px-3 py-2 text-sm"
                        value={name}
                        onChange={(e) => setName(e.target.value)}
                        autoFocus
                    />
                </label>
                <div className="flex justify-end gap-2">
                    <button
                        type="button"
                        className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                        onClick={onClose}
                    >
                        Cancel
                    </button>
                    <button
                        type="submit"
                        data-testid="save-name-submit"
                        className="rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium hover:bg-blue-500"
                    >
                        {submitLabel}
                    </button>
                </div>
            </form>
        </Modal>
    );
}
