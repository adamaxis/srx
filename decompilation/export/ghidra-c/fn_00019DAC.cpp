// Ghidra headless export
// Entry: 0x00019DAC
// Name: FUN_00019dac
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00019dac(void)

{
  *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) =
       *(byte *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 1) |
       *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102);
  FUN_000566d8();
  return;
}


