import { SectionHeading } from '@/components/ui/SectionHeading';

import { labelClass, readOnlyClass } from './formStyles';

export interface ReadOnlySourceField {
    label: string;
    value: string;
    testId?: string;
}

export interface ReadOnlySourceSectionProps {
    title?: string;
    fields: ReadOnlySourceField[];
    testId?: string;
}

export function ReadOnlySourceSection({
    title = 'Source',
    fields,
    testId = 'read-only-source',
}: ReadOnlySourceSectionProps) {
    return (
        <section data-testid={testId}>
            <SectionHeading>{title}</SectionHeading>
            <div className="grid gap-2 sm:grid-cols-3">
                {fields.map((field) => (
                    <label key={field.label} className={`block ${labelClass}`}>
                        {field.label}
                        <input
                            className={readOnlyClass}
                            readOnly
                            value={field.value}
                            data-testid={field.testId}
                        />
                    </label>
                ))}
            </div>
        </section>
    );
}
