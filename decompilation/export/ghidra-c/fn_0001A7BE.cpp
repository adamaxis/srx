// Ghidra headless export
// Entry: 0x0001A7BE
// Name: FUN_0001a7be
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE FUN_000013be
//   0x000061CC append_sprite_record


void FUN_0001a7be(void)

{
  append_sprite_record();
  DAT_00ffe07a = DAT_00ffe07a + 0x60;
  if (0x127f < DAT_00ffe07a) {
    return;
  }
  FUN_000013be();
  return;
}


