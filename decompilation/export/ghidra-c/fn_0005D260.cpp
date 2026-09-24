// Ghidra headless export
// Entry: 0x0005D260
// Name: FUN_0005d260
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0005d260(void)

{
  short sVar1;
  undefined4 *puVar2;
  
  sVar1 = 0x10;
  puVar2 = (undefined4 *)&DAT_00fff11e;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  DAT_00fff11e = 0x11c0;
  DAT_00fff122 = 0xfe80;
  DAT_00fff13e._2_2_ = 0xffff;
  DAT_00fff142._0_2_ = 0xb4;
  return;
}


