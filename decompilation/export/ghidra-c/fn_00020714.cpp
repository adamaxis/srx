// Ghidra headless export
// Entry: 0x00020714
// Name: finalize_mode_exit
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001F246 FUN_0001f246


void finalize_mode_exit(void)

{
  DAT_00fff10c = 0;
  FUN_0001f246();
  return;
}


