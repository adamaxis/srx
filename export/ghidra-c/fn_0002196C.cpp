// Ghidra headless export
// Entry: 0x0002196C
// Name: mark_effect_bit
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void mark_effect_bit(void)

{
  DAT_00fffc1e = DAT_00fffc1e | 1 << (_DAT_00ffdd64 & 0x1f);
  return;
}


