// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000FDFE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000fdfe(void)

{
  char cVar1;
  ushort uVar2;
  undefined1 *puVar3;
  short *psVar4;
  
  DAT_00fff0da = DAT_00fff0da + -1;
  if (DAT_00fff0da < '\0') {
    DAT_00fff0da = '\x06';
    cVar1 = DAT_00fff0db + -3;
    DAT_00fff0db = DAT_00fff0db + '\x01';
    if (-1 < cVar1) {
      DAT_00fff0db = '\0';
    }
    _DAT_00ffdd02 = 1;
    FUN_00001378();
    DAT_00ffd989 = 0;
    uVar2 = (ushort)DAT_00fff0d0;
    for (puVar3 = &DAT_00ffd986; *(ushort *)(puVar3 + 6) = uVar2 * 0x20 + *(short *)(puVar3 + 6),
        puVar3[3] != '\0'; puVar3 = puVar3 + 8) {
    }
    uVar2 = (ushort)DAT_00fff0d1;
    for (psVar4 = (short *)&DAT_00ffd986; *psVar4 = uVar2 * 0x18 + *psVar4,
        *(char *)((int)psVar4 + 3) != '\0'; psVar4 = psVar4 + 4) {
    }
    _DAT_00ffd980 = 1;
    DAT_00ffef14 = DAT_00ffef14 | 2;
  }
  FUN_0000fdae();
  return;
}


