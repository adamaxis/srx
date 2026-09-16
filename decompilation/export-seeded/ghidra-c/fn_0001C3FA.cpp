// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001C3FA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c3fa(void)

{
  ushort uVar1;
  int extraout_A1;
  int extraout_A1_00;
  int unaff_A6;
  undefined4 uVar2;
  undefined4 uVar3;
  
  if (_DAT_00ffdf8c != 0) {
    FUN_0001c506();
    return;
  }
  if ((DAT_00ff158c < (char)*_DAT_00ff1588) &&
     (DAT_00ff1582 = DAT_00ff1582 - 1, (short)((ushort)DAT_00ff1582 << 8) < 0)) {
    DAT_00ff1582 = 4;
    if ((char)(DAT_00ff158c + '\x01') <= (char)*_DAT_00ff1588) {
      DAT_00ff158c = DAT_00ff158c + '\x01';
    }
  }
  if ((DAT_00ff158d < (char)*_DAT_00ff1584) &&
     (bRam00ff1583 = bRam00ff1583 - 1, (short)((ushort)bRam00ff1583 << 8) < 0)) {
    bRam00ff1583 = 4;
    if ((char)(DAT_00ff158d + '\x01') <= (char)*_DAT_00ff1584) {
      DAT_00ff158d = DAT_00ff158d + '\x01';
    }
  }
  uVar2 = *(undefined4 *)((int)_DAT_00ff1584 + (short)((short)DAT_00ff158d << 2) + 2);
  uVar3 = *(undefined4 *)((int)_DAT_00ff1588 + (short)((short)DAT_00ff158c << 2) + 2);
  FUN_00001378(uVar2);
  if ((*(byte *)(extraout_A1 + 0x12) & 2) != 0) {
    uVar1 = (short)DAT_00ffe169 - 1;
    if (3 < (short)uVar1) {
      uVar1 = 3;
    }
    unaff_A6 = *(int *)((int)&PTR_DAT_0001c5ec + (int)(short)((uVar1 & 3) << 2));
    if (*(short *)(unaff_A6 + (short)((_DAT_00ffdf88 & 3) << 1)) != 0) {
      FUN_00001378(uVar2);
    }
  }
  FUN_00001378(uVar3);
  if (((*(byte *)(extraout_A1_00 + 0x12) & 2) != 0) &&
     (*(short *)(unaff_A6 + (short)((_DAT_00ffdf88 & 3) << 1)) != 0)) {
    FUN_00001378(uVar3);
  }
  if ((_DAT_00ffe084 != 0) && (7 < (_DAT_00ffdf88 & 0xf))) {
    FUN_0001a570();
  }
  return;
}


