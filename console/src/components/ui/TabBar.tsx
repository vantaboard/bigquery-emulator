import { X } from 'lucide-react';

import { cn } from '@/lib/utils';

import type { TabBarProps } from './Tabs';

export function TabBar({
    tabs,
    activeId,
    onChange,
    onClose,
    onContextMenu,
    variant = 'underline',
    className,
}: TabBarProps) {
    return (
        <div
            className={cn(
                'flex gap-1 overflow-x-auto text-sm',
                variant === 'underline' && 'border-b border-[var(--bq-border)]',
                variant === 'workspace' && 'rounded-md border border-[var(--bq-border)] bg-black/20 p-1',
                className,
            )}
            role="tablist"
        >
            {tabs.map((tab) => {
                const active = tab.id === activeId;
                const closable = tab.closable === true && onClose;

                return (
                    <div
                        key={tab.id}
                        className={cn(
                            'flex shrink-0 items-center',
                            variant === 'workspace' &&
                                cn(
                                    'rounded border border-transparent',
                                    active && 'border-[var(--bq-border)] bg-[var(--bq-surface)]',
                                ),
                        )}
                        onContextMenu={
                            onContextMenu
                                ? (event) => {
                                      event.preventDefault();
                                      onContextMenu(tab.id, event);
                                  }
                                : undefined
                        }
                    >
                        <button
                            type="button"
                            role="tab"
                            aria-selected={active}
                            data-testid={tab.testId}
                            className={cn(
                                variant === 'underline' && '-mb-px border-b-2 px-3 py-2',
                                variant === 'underline' &&
                                    (active
                                        ? 'border-blue-500 text-white'
                                        : 'border-transparent text-[var(--bq-muted)] hover:text-white'),
                                variant === 'workspace' && 'rounded px-3 py-1.5',
                                variant === 'workspace' &&
                                    (active ? 'text-white' : 'text-[var(--bq-muted)] hover:bg-white/5 hover:text-white'),
                            )}
                            onClick={() => onChange(tab.id)}
                        >
                            <span className="inline-flex items-center gap-1.5">
                                {tab.icon ? (
                                    <span
                                        className={cn(
                                            'inline-flex shrink-0',
                                            variant === 'workspace' && '[&_svg]:size-3.5',
                                            active ? 'opacity-100' : 'opacity-70',
                                        )}
                                    >
                                        {tab.icon}
                                    </span>
                                ) : null}
                                {tab.label}
                                {tab.badge ? (
                                    <span className="text-xs text-[var(--bq-muted)]">{tab.badge}</span>
                                ) : null}
                            </span>
                        </button>
                        {closable ? (
                            <button
                                type="button"
                                className={cn(
                                    'rounded p-1 text-[var(--bq-muted)] hover:bg-white/10 hover:text-white',
                                    variant === 'workspace' && 'mr-0.5',
                                )}
                                aria-label={`Close ${typeof tab.label === 'string' ? tab.label : 'tab'}`}
                                onClick={(event) => {
                                    event.stopPropagation();
                                    onClose(tab.id);
                                }}
                            >
                                <X className="size-3.5" />
                            </button>
                        ) : null}
                    </div>
                );
            })}
        </div>
    );
}
