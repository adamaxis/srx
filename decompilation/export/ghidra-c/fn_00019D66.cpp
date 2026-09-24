// Ghidra headless export
// Entry: 0x00019D66
// Name: FUN_00019d66
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017998 FUN_00017998
//   0x00017A5E FUN_00017a5e
//   0x00017C52 FUN_00017c52
//   0x00017C84 FUN_00017c84
//   0x00017E04 FUN_00017e04
//   0x00019DAC FUN_00019dac
//   0x0001B706 FUN_0001b706


void FUN_00019d66(void)

{
  if (*(char *)((int)&DAT_00fffb30 + (int)DAT_00fff102) < '\x01') {
    *(undefined1 *)((int)&DAT_00fffb30 + (int)DAT_00fff102) = 0;
    FUN_00017c84();
    FUN_00017c52();
    FUN_00017e04();
    FUN_0001b706();
    return;
  }
  FUN_00017c84();
  FUN_00017c52();
  FUN_00019dac();
  FUN_00017998();
  FUN_00017a5e();
  return;
}


