import { cn } from '@/lib/utils';

export interface UnplannedTabProps {
    title?: string;
    message?: string;
    className?: string;
}

export function UnplannedTab({
    title = 'Coming soon',
    message = 'This view is not planned yet.',
    className,
}: UnplannedTabProps) {
    return (
        <div
            className={cn(
                'flex min-h-48 flex-col items-center justify-center gap-2 p-8 text-center text-sm text-[var(--bq-muted)]',
                className,
            )}
        >
            <p className="font-medium text-white">{title}</p>
            <p>{message}</p>
        </div>
    );
}
