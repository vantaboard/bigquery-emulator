import type { LucideIcon } from 'lucide-react';
import { ChevronDown } from 'lucide-react';
import { useEffect, useId, useRef, useState } from 'react';

import { cn } from '@/lib/utils';

const VARIANT_CLASS = {
    primary: 'bg-blue-600 text-white hover:bg-blue-500 disabled:hover:bg-blue-600',
    secondary: 'border border-[var(--bq-border)] hover:bg-white/5 disabled:hover:bg-transparent',
    danger: 'border border-red-500/40 text-red-300 hover:bg-red-500/10 disabled:hover:bg-transparent',
} as const;

export type ToolbarButtonVariant = keyof typeof VARIANT_CLASS;

export interface ToolbarMenuItem {
    label: string;
    onClick: () => void;
    disabled?: boolean;
}

export interface ToolbarButtonProps {
    icon: LucideIcon;
    label: string;
    variant?: ToolbarButtonVariant;
    disabled?: boolean;
    onClick?: () => void;
    dropdown?: ToolbarMenuItem[];
    className?: string;
}

export function ToolbarButton({
    icon: Icon,
    label,
    variant = 'secondary',
    disabled = false,
    onClick,
    dropdown,
    className,
}: ToolbarButtonProps) {
    const menuId = useId();
    const rootRef = useRef<HTMLDivElement>(null);
    const [open, setOpen] = useState(false);

    useEffect(() => {
        if (!open) return;

        const onPointerDown = (event: MouseEvent) => {
            if (!rootRef.current?.contains(event.target as Node)) {
                setOpen(false);
            }
        };

        document.addEventListener('mousedown', onPointerDown);
        return () => document.removeEventListener('mousedown', onPointerDown);
    }, [open]);

    const baseClass = cn(
        'inline-flex items-center gap-2 rounded-md px-3 py-1.5 text-sm disabled:cursor-not-allowed disabled:opacity-50',
        VARIANT_CLASS[variant],
        className,
    );

    if (!dropdown?.length) {
        return (
            <button type="button" className={baseClass} disabled={disabled} onClick={onClick}>
                <Icon className="size-4 shrink-0" />
                {label}
            </button>
        );
    }

    return (
        <div ref={rootRef} className="relative inline-flex">
            <button
                type="button"
                className={cn(baseClass, 'rounded-r-none pr-2')}
                disabled={disabled}
                onClick={onClick}
            >
                <Icon className="size-4 shrink-0" />
                {label}
            </button>
            <button
                type="button"
                className={cn(baseClass, 'rounded-l-none border-l border-black/20 px-2')}
                disabled={disabled}
                aria-haspopup="menu"
                aria-expanded={open}
                aria-controls={menuId}
                onClick={() => setOpen((value) => !value)}
            >
                <ChevronDown className="size-4 shrink-0" />
            </button>
            {open ? (
                <div
                    id={menuId}
                    role="menu"
                    className="absolute right-0 top-full z-20 mt-1 min-w-40 rounded-md border border-[var(--bq-border)] bg-[var(--bq-surface)] py-1 shadow-lg"
                >
                    {dropdown.map((item) => (
                        <button
                            key={item.label}
                            type="button"
                            role="menuitem"
                            className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5 disabled:cursor-not-allowed disabled:opacity-50"
                            disabled={item.disabled}
                            onClick={() => {
                                item.onClick();
                                setOpen(false);
                            }}
                        >
                            {item.label}
                        </button>
                    ))}
                </div>
            ) : null}
        </div>
    );
}
