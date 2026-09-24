// Ghidra headless export
// Entry: 0x0001A106
// Name: FUN_0001a106
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 FUN_00017c52
//   0x00017C84 FUN_00017c84
//   0x0001A154 FUN_0001a154
//   0x0001B706 FUN_0001b706


void FUN_0001a106(void)

{
  if (DAT_00fff0aa < '\x01') {
    FUN_0001b706();
  }
  else {
    FUN_0001a154();
  }
  FUN_00017c84();
  FUN_00017c52();
  return;
}


