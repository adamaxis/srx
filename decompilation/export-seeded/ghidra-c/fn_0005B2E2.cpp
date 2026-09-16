// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B2E2


void FUN_0005b2e2(void)

{
  short in_D0w;
  ushort uVar1;
  short sVar2;
  ushort *puVar3;
  
  sVar2 = 0xe;
  puVar3 = (ushort *)(&DAT_00ffd87e + (short)(in_D0w * 0x20 + 2));
  do {
    uVar1 = *puVar3;
    if ((uVar1 & 0xe) != 0) {
      uVar1 = uVar1 - 2;
    }
    if ((uVar1 & 0xe0) != 0) {
      uVar1 = uVar1 - 0x20;
    }
    if ((uVar1 & 0xe00) != 0) {
      uVar1 = uVar1 - 0x200;
    }
    *puVar3 = uVar1;
    sVar2 = sVar2 + -1;
    puVar3 = puVar3 + 1;
  } while (sVar2 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


