// Ghidra headless export
// Entry: 0x0001D1F4
// Name: FUN_0001d1f4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00000E9C make_vdp_transfer_command
//   0x0001A01E FUN_0001a01e
//   0x0001D1DC FUN_0001d1dc
//   0x0001D3F4 FUN_0001d3f4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001d1f4(void)

{
  ushort uVar2;
  uint uVar1;
  short sVar3;
  int in_A0;
  int extraout_A1;
  
  DAT_00ffe008 = DAT_00ffe008 + -1;
  if (-1 < DAT_00ffe008) {
    if ((DAT_00ffdf88 & 3) == 0) {
      FUN_0001d1dc();
    }
    sVar3 = 0x46;
    do {
      uVar2 = sample_source_random_bound();
      make_vdp_transfer_command();
      uVar1 = FUN_0001d3f4();
      Ram00c00000 = *(uint *)(extraout_A1 + (short)(uVar2 & 0xfffc)) & uVar1;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    return;
  }
  *(byte *)(in_A0 + 0x12) = *(byte *)(in_A0 + 0x12) & 0xef;
  FUN_0001d1dc();
  if (DAT_00ffe16b != '\0') {
    DAT_00ffe0ae = 0;
    DAT_00ffe0b0 = 4;
    _DAT_00ffdf8c = 1;
    DAT_00ffdf76 = 0x70;
    DAT_00ffdf78 = 0xaf;
    DAT_00ffdf72 = 0xaf;
    DAT_00ffdf7a = &UNK_0001d428;
    DAT_00ffdf7e = &UNK_000da038;
    DAT_00ffdf74 = 0x811b;
    DAT_00ffdf70 = 0x51;
    DAT_00ffdf82 = 0;
    return;
  }
  FUN_0001a01e();
  return;
}


