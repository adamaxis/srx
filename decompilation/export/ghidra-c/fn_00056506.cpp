// Ghidra headless export
// Entry: 0x00056506
// Name: FUN_00056506
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019BF6 FUN_00019bf6
//   0x00019C5C FUN_00019c5c
//   0x00055F44 FUN_00055f44
//   0x000562DE FUN_000562de
//   0x00056632 FUN_00056632


void FUN_00056506(void)

{
  char cVar1;
  byte bVar2;
  ushort uVar3;
  uint uVar4;
  short extraout_D1w;
  short unaff_D3w;
  ushort uVar5;
  uint uVar6;
  undefined4 unaff_D6;
  short sVar7;
  short unaff_D7w;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A1;
  
  cVar1 = *(char *)(in_A0 + 0x1a + (int)unaff_D3w);
  if (-1 < (char)(cVar1 + -4)) {
    if (cVar1 == '\x04') {
      FUN_00019c5c();
      FUN_00056632();
      return;
    }
    if ((char)(*(char *)(in_A0 + 0x1a + (int)unaff_D3w) + -8) < '\0') {
      FUN_00019c5c();
      return;
    }
  }
  FUN_00055f44();
  cVar1 = DAT_00fffbbe + (char)unaff_D6;
  FUN_00019c5c();
  sVar7 = (short)CONCAT31((int3)((uint)unaff_D6 >> 8),cVar1);
  if ((short)(unaff_D7w - sVar7) < 0) {
    sVar7 = unaff_D7w;
  }
  sVar7 = unaff_D7w + sVar7;
  if (sVar7 == 0) {
    sVar7 = 1;
  }
  cVar1 = *(char *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 1) + -1;
  bVar2 = (char)sVar7 - cVar1;
  uVar3 = CONCAT11((char)((ushort)sVar7 >> 8),bVar2);
  uVar6 = (uint)uVar3;
  if (bVar2 == 0 || SBORROW1((char)sVar7,cVar1) != (short)((ushort)bVar2 << 8) < 0) {
    return;
  }
  FUN_000562de();
  FUN_00056632();
  if (*(char *)(extraout_A0 + 0x1a + (int)unaff_D3w) == '\v') {
    uVar6 = (uint)*(byte *)(extraout_A1 + 6 + (int)extraout_D1w);
    uVar5 = (ushort)DAT_00fffbc9;
  }
  else {
    FUN_00019bf6();
    if ((char)(*(char *)(extraout_A0_00 + 0x1a + (int)unaff_D3w) + -4) < '\0') {
      uVar5 = (ushort)DAT_00fffbc7;
    }
    else {
      uVar5 = (ushort)DAT_00fffbc8;
    }
  }
  FUN_000562de();
  if ((short)(uVar5 - 2) < 0) {
    uVar5 = 2;
  }
  if ((ushort)(7 - uVar5) == 0 || 7 < uVar5) {
    uVar4 = 10;
  }
  else {
    uVar4 = (uint)(ushort)(7 - uVar5) * 0x14;
  }
  if (-1 < (short)(uVar3 - (short)((uVar6 * (uVar4 & 0xffff)) / 100))) {
    return;
  }
  return;
}


