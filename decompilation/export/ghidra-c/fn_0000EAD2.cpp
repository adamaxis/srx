// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000EAD2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ead2(void)

{
  short sVar1;
  ushort uVar2;
  short extraout_D1w;
  short unaff_D3w;
  short unaff_D5w;
  short unaff_D6w;
  
  sVar1 = _DAT_00ffdfe4;
  uVar2 = FUN_0000edfa();
  uVar2 = uVar2 & 0x38;
  if (uVar2 == 0x20) {
    if ((ushort)-(extraout_D1w - unaff_D5w) < (ushort)(unaff_D3w - unaff_D6w)) {
      return;
    }
  }
  else if (uVar2 == 0x28) {
    if ((ushort)-(sVar1 - unaff_D6w) <= (ushort)-(extraout_D1w - unaff_D5w)) {
      return;
    }
  }
  else if (uVar2 == 0x38) {
    if ((ushort)(unaff_D3w - unaff_D6w) <= (ushort)-(extraout_D1w - unaff_D5w)) {
      return;
    }
  }
  else if ((ushort)-(extraout_D1w - unaff_D5w) < (ushort)-(sVar1 - unaff_D6w)) {
    return;
  }
  return;
}


