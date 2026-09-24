// Ghidra headless export
// Entry: 0x0005DAC4
// Name: fill_boot_horizontal_scroll
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 FUN_00000d44
//   0x0005DAFC FUN_0005dafc


void fill_boot_horizontal_scroll(void)

{
  ushort uVar1;
  short sVar2;
  ushort *extraout_A0;
  
  sVar2 = 0xdf;
  do {
    uVar1 = FUN_00000d44();
    *extraout_A0 = uVar1 >> 5 & 0x1f;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  FUN_0005dafc();
  return;
}


