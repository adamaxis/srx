// Ghidra headless export
// Entry: 0x0000E8A0
// Name: FUN_0000e8a0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


byte FUN_0000e8a0(void)

{
  short sVar1;
  byte bVar2;
  ushort unaff_D5w;
  ushort unaff_D6w;
  int in_A0;
  
  DAT_00ffdd28._0_1_ = 0;
  if ((*(byte *)(in_A0 + 0x10) & 4) != 0) {
    DAT_00ffdfe4 = ((ushort)(unaff_D5w - 0xc) >> 5) + (unaff_D6w - 7 & 0x7fe0) * 2;
    DAT_00ffef10 = (&DAT_00ff1700)[DAT_00ffdfe4];
    if (DAT_00ffef10 == 0x7f) {
      DAT_00ffdd28._0_1_ = DAT_00ffef10;
    }
    sVar1 = ((ushort)(unaff_D5w + 0xc) >> 5) + (unaff_D6w - 7 & 0x7fe0) * 2;
    DAT_00ffef11 = (&DAT_00ff1700)[sVar1];
    if (DAT_00ffef11 == 0x7f) {
      DAT_00ffdd28._0_1_ = DAT_00ffef11;
    }
    bVar2 = DAT_00ffef10;
    if (DAT_00ffef10 <= DAT_00ffef11) {
      bVar2 = DAT_00ffef11;
      DAT_00ffdfe4 = sVar1;
    }
    sVar1 = ((ushort)(unaff_D5w - 0xc) >> 5) + (unaff_D6w + 7 & 0x7fe0) * 2;
    DAT_00ffef12 = (&DAT_00ff1700)[sVar1];
    if (DAT_00ffef12 == 0x7f) {
      DAT_00ffdd28._0_1_ = DAT_00ffef12;
    }
    if (bVar2 <= DAT_00ffef12) {
      bVar2 = DAT_00ffef12;
      DAT_00ffdfe4 = sVar1;
    }
    sVar1 = ((ushort)(unaff_D5w + 0xc) >> 5) + (unaff_D6w + 7 & 0x7fe0) * 2;
    DAT_00ffef13 = (&DAT_00ff1700)[sVar1];
    if (DAT_00ffef13 == 0x7f) {
      DAT_00ffdd28._0_1_ = DAT_00ffef13;
    }
    if (bVar2 <= DAT_00ffef13) {
      bVar2 = DAT_00ffef13;
      DAT_00ffdfe4 = sVar1;
    }
    if (DAT_00ffdd28._0_1_ != 0) {
      bVar2 = DAT_00ffdd28._0_1_;
    }
    DAT_00ffdd28._1_1_ = 0;
    return bVar2;
  }
  DAT_00ffdfe4 = (unaff_D5w >> 5) + (unaff_D6w & 0x7fe0) * 2;
  DAT_00ffef10 = (&DAT_00ff1700)[DAT_00ffdfe4];
  DAT_00ffdd28._1_1_ = 0;
  DAT_00ffdd28._0_1_ = 0;
  return (&DAT_00ff1700)[DAT_00ffdfe4];
}


