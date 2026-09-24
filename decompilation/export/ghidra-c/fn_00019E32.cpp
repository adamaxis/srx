// Ghidra headless export
// Entry: 0x00019E32
// Name: FUN_00019e32
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 FUN_00017c52
//   0x00017C84 set_hud_selection_value
//   0x00017E04 FUN_00017e04
//   0x0001B706 FUN_0001b706
//   0x00056434 FUN_00056434


void FUN_00019e32(void)

{
  set_hud_selection_value();
  FUN_00017c52();
  if ('\0' < DAT_00fff0d5) {
    FUN_0001b706();
    FUN_00056434();
    FUN_00017e04();
  }
  FUN_0001b706();
  return;
}


