import {
    Camera,
    Database,
    Eye,
    FunctionSquare,
    Globe,
    Layers,
    Search,
    Table2,
    type LucideIcon,
} from 'lucide-react';

import type { WorkspaceTab } from '@/features/workspace/types';
import { cn } from '@/lib/utils';
import type { ResourceType } from '@/types/api';

export type ResourceIconKind =
    | { kind: 'query' }
    | { kind: 'dataset' }
    | { kind: 'routine' }
    | { kind: 'table'; resourceType?: ResourceType };

function tableIcon(resourceType: ResourceType | undefined): LucideIcon {
    switch (resourceType) {
        case 'VIEW':
            return Eye;
        case 'MATERIALIZED_VIEW':
            return Layers;
        case 'SNAPSHOT':
            return Camera;
        case 'EXTERNAL':
            return Globe;
        case 'TABLE':
        default:
            return Table2;
    }
}

function iconForKind(kind: ResourceIconKind): LucideIcon {
    switch (kind.kind) {
        case 'query':
            return Search;
        case 'dataset':
            return Database;
        case 'routine':
            return FunctionSquare;
        case 'table':
            return tableIcon(kind.resourceType);
    }
}

export function resourceIconForTab(tab: WorkspaceTab): ResourceIconKind {
    switch (tab.type) {
        case 'query':
            return { kind: 'query' };
        case 'dataset':
            return { kind: 'dataset' };
        case 'routine':
            return { kind: 'routine' };
        case 'table':
            return { kind: 'table', resourceType: tab.resourceType };
    }
}

export function ResourceIcon({
    kind,
    className,
}: {
    kind: ResourceIconKind;
    className?: string;
}) {
    const Icon = iconForKind(kind);
    return <Icon aria-hidden className={cn('size-4 shrink-0', className)} />;
}
