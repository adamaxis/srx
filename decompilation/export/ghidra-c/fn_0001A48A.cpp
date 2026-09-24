// Ghidra headless export
// Entry: 0x0001A48A
// Name: FUN_0001a48a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE FUN_000013be
//   0x000061CC append_sprite_record


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a48a(void)

{
  DAT_00ffe0d2 = DAT_00ffe0d2 + -1;
  if (DAT_00ffe0d2 < 0) {
    DAT_00ffe0d2 = 2;
    DAT_00ffe0d0 = DAT_00ffe0d0 + 1;
    if (2 < DAT_00ffe0d0) {
      _DAT_00ffe0d4 = 0;
      return;
    }
  }
  append_sprite_record();
  FUN_000013be();
  return;
}


