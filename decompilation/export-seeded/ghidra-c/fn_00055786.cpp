// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055786


void FUN_00055786(void)

{
  int in_A0;
  int extraout_A0;
  int iVar1;
  int extraout_A0_00;
  
  if ((*(byte *)(in_A0 + 0xdf) & 1) == 0) {
    if ((*(byte *)(in_A0 + 0xdf) & 2) == 0) {
      return;
    }
    FUN_00055828();
    iVar1 = extraout_A0_00;
  }
  else {
    FUN_00055828();
    iVar1 = extraout_A0;
  }
  if ((short)(*(short *)(iVar1 + 0x3e) + -3) < 0) {
    FUN_0001047e();
  }
  FUN_00000dba();
  return;
}


