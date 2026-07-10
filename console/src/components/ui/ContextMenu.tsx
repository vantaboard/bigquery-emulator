import { useEffect, useLayoutEffect, useRef, useState } from 'react';
import type { LucideIcon } from 'lucide-react';

import { cn } from '@/lib/utils';

export interface ContextMenuItem {
    label: string;
    icon?: LucideIcon;
    disabled?: boolean;
    onClick: () => void;
}

export interface ContextMenuState {
    x: number;
    y: number;
}

interface ContextMenuProps {
    open: ContextMenuState | null;
    items: ContextMenuItem[];
    onClose: () => void;
}

export function ContextMenu({ open, items, onClose }: ContextMenuProps) {
    const menuRef = useRef<HTMLDivElement>(null);
    const [position, setPosition] = useState<{ x: number; y: number } | null>(null);

    useLayoutEffect(() => {
        if (!open) {
            setPosition(null);
            return;
        }
        const menu = menuRef.current;
        if (!menu) {
            setPosition({ x: open.x, y: open.y });
            return;
        }
        const rect = menu.getBoundingClientRect();
        const padding = 8;
        const maxX = window.innerWidth - rect.width - padding;
        const maxY = window.innerHeight - rect.height - padding;
        setPosition({
            x: Math.max(padding, Math.min(open.x, maxX)),
            y: Math.max(padding, Math.min(open.y, maxY)),
        });
    }, [open, items]);

    useEffect(() => {
        if (!open) return;

        const onKeyDown = (event: KeyboardEvent) => {
            if (event.key === 'Escape') onClose();
        };
        const onPointerDown = (event: MouseEvent) => {
            const target = event.target as Node;
            if (menuRef.current?.contains(target)) return;
            onClose();
        };

        window.addEventListener('keydown', onKeyDown);
        window.addEventListener('mousedown', onPointerDown);
        return () => {
            window.removeEventListener('keydown', onKeyDown);
            window.removeEventListener('mousedown', onPointerDown);
        };
    }, [open, onClose]);

    if (!open) return null;

    return (
        <div
            ref={menuRef}
            role="menu"
            data-testid="context-menu"
            className="fixed z-50 min-w-48 rounded-md border border-[var(--bq-border)] bg-[var(--bq-surface)] py-1 shadow-lg"
            style={{
                left: position?.x ?? open.x,
                top: position?.y ?? open.y,
            }}
        >
            {items.map((item) => {
                const Icon = item.icon;
                return (
                    <button
                        key={item.label}
                        type="button"
                        role="menuitem"
                        disabled={item.disabled}
                        className={cn(
                            'flex w-full items-center gap-2 px-3 py-1.5 text-left text-sm hover:bg-white/5',
                            item.disabled && 'cursor-not-allowed opacity-50',
                        )}
                        onClick={() => {
                            if (item.disabled) return;
                            item.onClick();
                            onClose();
                        }}
                    >
                        {Icon ? <Icon aria-hidden className="size-4 shrink-0" /> : null}
                        {item.label}
                    </button>
                );
            })}
        </div>
    );
}
