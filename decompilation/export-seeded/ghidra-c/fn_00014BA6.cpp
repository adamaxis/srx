// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014BA6


void FUN_00014ba6(void)

{
  ushort uVar1;
  short *in_A0;
  
  if ((*(byte *)((int)in_A0 + 0xb1) & 1) == 0) {
    if (-1 < (short)(in_A0[0x4b] - *in_A0)) {
LAB_00014bce:
      *(byte *)((int)in_A0 + 0xb1) = *(byte *)((int)in_A0 + 0xb1) | 4;
    }
  }
  else if ((short)(in_A0[0x4b] - *in_A0) < 0) goto LAB_00014bce;
  if ((*(byte *)((int)in_A0 + 0xb1) & 2) == 0) {
    if ((short)(in_A0[0x4c] - in_A0[2]) < 0) goto LAB_00014bf4;
  }
  else if (-1 < (short)(in_A0[0x4c] - in_A0[2])) goto LAB_00014bf4;
  *(byte *)((int)in_A0 + 0xb1) = *(byte *)((int)in_A0 + 0xb1) | 8;
LAB_00014bf4:
  uVar1 = in_A0[0x5c];
  if (-1 < (short)uVar1) {
    if ((*(byte *)((int)in_A0 + 0xb1) & 4) == 0) {
      if (((*(byte *)((int)in_A0 + 0xb1) & 8) == 0) || ((uVar1 & 2) == 0)) {
        return;
      }
    }
    else if ((uVar1 & 2) != 0) {
      return;
    }
  }
  *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) | 2;
  FUN_00014682();
  return;
}


