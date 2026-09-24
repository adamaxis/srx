// Ghidra headless export
// Entry: 0x00014FF8
// Name: FUN_00014ff8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00015118 FUN_00015118
//   0x00015D3C FUN_00015d3c
//   0x00055640 FUN_00055640


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014ff8(void)

{
  ushort uVar1;
  short in_D0w;
  ushort uVar2;
  short sVar3;
  int unaff_D3;
  short *in_A0;
  short *in_A1;
  short *extraout_A1;
  
  FUN_00015118();
  if (_DAT_00ffdfd8 == 0) {
    FUN_00055640();
  }
  in_A1[0x16] = 0x4000;
  *in_A1 = *in_A0;
  in_A1[2] = in_A0[2];
  in_A1[0x17] = in_A0[0x17];
  *(undefined1 *)((int)in_A1 + 0x9d) = *(undefined1 *)((int)in_A0 + 0x9d);
  *(undefined1 *)((int)in_A1 + 0x57) = *(undefined1 *)((int)in_A0 + 0x57);
  *(undefined1 *)(in_A1 + 0x2b) = *(undefined1 *)(in_A0 + 0x2b);
  in_A1[0x1f] = in_D0w;
  in_A1[0xb] = 0x14;
  *(short **)(in_A1 + 0x30) = in_A0;
  if ((*(byte *)(in_A0 + 8) & 8) != 0) {
    *(byte *)(in_A1 + 8) = *(byte *)(in_A1 + 8) | 8;
  }
  if ((unaff_D3 != 0) && ((*(byte *)(unaff_D3 + 0x10) & 1) != 0)) {
    if (_DAT_00ffdfd8 == 0) {
      *(short **)(unaff_D3 + 0xac) = in_A0;
    }
    sVar3 = *(short *)(unaff_D3 + 4);
    FUN_00015d3c();
    extraout_A1[0x17] = sVar3;
    in_A1 = extraout_A1;
  }
  uVar1 = in_A1[0x17];
  uVar2 = uVar1 << 3 | uVar1 >> 0xd;
  if ((short)uVar2 < 0) {
    uVar2 = uVar2 + 1;
  }
  sVar3 = (uVar2 & 7) * 4;
  DAT_00ffdd60 = *in_A1 + *(short *)(&DAT_00015a6c + sVar3);
  DAT_00ffdd62 = in_A1[2] + *(short *)(&DAT_00015a6c + (short)(sVar3 + 2));
  *in_A1 = *(short *)(&DAT_00015a4c + sVar3) + *in_A1;
  in_A1[2] = *(short *)(&DAT_00015a4c + (short)(sVar3 + 2)) + in_A1[2];
  uVar1 = uVar1 >> 8 & 0xfffe;
  sVar3 = *(short *)((int)&DAT_00015e16 + (int)(short)uVar1);
  in_A1[0x26] = (short)(*(short *)((int)&DAT_00015e56 + (int)(short)uVar1) * 0xc >> 4);
  in_A1[0x27] = (short)(sVar3 * 0xc >> 4);
  return;
}


