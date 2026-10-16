// Ghidra headless export
// Entry: 0x0001A8F4
// Name: reset_effect_entity
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)
//
// ROM source audit: clears FFE082, sets FFE080=1, and clears the word at
// FF163E+0x0C.  The generated `in_A0` is the A0 value established by the
// callers at 0x1A88E/0x1AA40; the typed host conversion names those three
// fields in WorldEffectResetState.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void reset_effect_entity(void)

{
  int in_A0;
  
  _DAT_00ffe082 = 0;
  _DAT_00ffe080 = 1;
  *(undefined2 *)(in_A0 + 0xc) = 0;
  return;
}


