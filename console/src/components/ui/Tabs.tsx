import type { ReactNode } from 'react';

export interface TabItem {
    id: string;
    label: ReactNode;
    badge?: ReactNode;
    closable?: boolean;
    testId?: string;
}

export type TabVariant = 'underline' | 'workspace';

export interface TabBarProps {
    tabs: TabItem[];
    activeId: string;
    onChange: (id: string) => void;
    onClose?: (id: string) => void;
    variant?: TabVariant;
    className?: string;
}

export { TabBar } from './TabBar';
