// Ghidra headless export
// Entry: 0x00011190
// Name: FUN_00011190
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined8 FUN_00011190(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D6w;
  short sVar1;
  short *extraout_A1;
  short *psVar2;
  
  do {
    sVar1 = 2;
    FUN_00000e9c();
    psVar2 = extraout_A1;
    do {
      VDP_DATA = unaff_D6w | *psVar2 + 0x3e9U;
      sVar1 = sVar1 + -1;
      psVar2 = psVar2 + 1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


