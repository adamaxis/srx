-- lib/int68k.lua: 68000 integer helpers shared by every module (replaces the copies in combat.lua:6-14,
-- entity_motion.lua:25 and actor_ai.lua:949-963).
local I = {}
function I.u8(v)  return v & 0xFF end
function I.u16(v) return v & 0xFFFF end
function I.u32(v) return v & 0xFFFFFFFF end
function I.s8(v)  v = v & 0xFF;       return v >= 0x80 and v - 0x100 or v end
function I.s16(v) v = v & 0xFFFF;     return v >= 0x8000 and v - 0x10000 or v end
function I.s32(v) v = v & 0xFFFFFFFF; return v >= 0x80000000 and v - 0x100000000 or v end  -- was "signed_word_from_long"
function I.fixed_int_word(v) return (I.u32(v) >> 16) & 0xFFFF end                            -- 16.16 -> integer word
return I
