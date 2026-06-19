import type { ReactNode } from 'react';

import { cn } from '@/lib/utils';

export interface SectionHeadingProps {
    children: ReactNode;
    className?: string;
}

export function SectionHeading({ children, className }: SectionHeadingProps) {
    return <h3 className={cn('mb-1 text-sm font-medium', className)}>{children}</h3>;
}
