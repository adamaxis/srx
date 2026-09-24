// Ghidra headless export
// Entry: 0x00016396
// Name: FUN_00016396
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001088 FUN_00001088
//   0x000163DA FUN_000163da


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_00016396(void)

{
  ushort uVar1;
  
  uVar1 = FUN_000163da();
  if (_DAT_00ffdf3a != 0) {
    uVar1 = DAT_00ffdf3c & 1;
    if (uVar1 == 0) {
      uVar1 = FUN_00001088();
      DAT_00ffef14 = DAT_00ffef14 | 1;
    }
    DAT_00ffdf3c = DAT_00ffdf3c - 1;
    if ((short)DAT_00ffdf3c < 0) {
      _DAT_00ffdf3a = 0;
      _DAT_00ffdf3e = 0xffff;
    }
  }
  return uVar1;
}


