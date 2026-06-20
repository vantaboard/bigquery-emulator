import { describe, expect, it } from 'vitest';

import { codeUnitToUtf8ByteOffset, utf8ByteOffsetToCodeUnit } from './sqlTools';

describe('sqlTools offset helpers', () => {
    it('round-trips ASCII offsets', () => {
        const sql = 'SELECT * FROM t';
        expect(utf8ByteOffsetToCodeUnit(sql, 0)).toBe(0);
        expect(utf8ByteOffsetToCodeUnit(sql, 6)).toBe(6);
        expect(codeUnitToUtf8ByteOffset(sql, 6)).toBe(6);
    });

    it('handles multi-byte UTF-8 characters', () => {
        const sql = 'SELECT café';
        const cafeStart = sql.indexOf('café');
        const byteOffset = new TextEncoder().encode(sql.slice(0, cafeStart + 4)).length;
        const codeUnit = utf8ByteOffsetToCodeUnit(sql, byteOffset);
        expect(codeUnit).toBe(cafeStart + 4);
        expect(codeUnitToUtf8ByteOffset(sql, codeUnit)).toBe(byteOffset);
    });

    it('counts astral code points as two UTF-16 code units', () => {
        const sql = 'SELECT 😀';
        const emojiPos = sql.indexOf('😀');
        expect(emojiPos).toBe(7);
        const byteAtEmoji = new TextEncoder().encode(sql.slice(0, emojiPos)).length;
        expect(utf8ByteOffsetToCodeUnit(sql, byteAtEmoji)).toBe(7);
        expect(utf8ByteOffsetToCodeUnit(sql, new TextEncoder().encode(sql).length)).toBe(sql.length);
    });
});
