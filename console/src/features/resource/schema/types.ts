export interface SchemaFieldDraft {
    id: string;
    name: string;
    type: string;
    mode: string;
    description?: string;
    maxLength?: number;
    rangeElementType?: string;
    fields?: SchemaFieldDraft[];
}

export interface SchemaValidationError {
    path: string;
    message: string;
}
