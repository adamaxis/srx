// Ghidra headless export
// Entry: 0x0001B364
// Name: FUN_0001b364
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE FUN_000013be
//   0x000061CC append_sprite_record
//   0x0001C3FA FUN_0001c3fa


void FUN_0001b364(void)

{
  FUN_0001c3fa();
  DAT_00ffe0b0 = DAT_00ffe0b0 + -1;
  if (DAT_00ffe0b0 < 0) {
    DAT_00ffe0b0 = 0;
    DAT_00ffe0ae = DAT_00ffe0ae + 1;
    if (4 < DAT_00ffe0ae) {
      DAT_00ffe0ae = 0;
    }
  }
  append_sprite_record();
  FUN_000013be();
  FUN_000013be();
  append_sprite_record();
  FUN_000013be();
  FUN_000013be();
  return;
}


