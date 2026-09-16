// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000E18


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00000e18(void)

{
  short unaff_D7w;
  undefined2 *extraout_A0;
  undefined2 *puVar1;
  
  FUN_00000e9c();
  puVar1 = extraout_A0;
  do {
    _DAT_00c00000 = *puVar1;
    unaff_D7w = unaff_D7w + -1;
    puVar1 = puVar1 + 1;
  } while (unaff_D7w != -1);
  return;
}


