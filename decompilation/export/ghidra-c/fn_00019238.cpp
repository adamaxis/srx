// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019238


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00019238(void)

{
  short extraout_D1w;
  short unaff_D6w;
  short unaff_D7w;
  short sVar1;
  undefined1 *puVar2;
  
  FUN_00018a92();
  if ((&DAT_00fffbcc)[unaff_D7w] != '\0') {
    _DAT_00ffdd50 = 0;
    DAT_00fff0dc = 0;
    FUN_00018998();
    FUN_00018a92();
    puVar2 = &DAT_00fffbbe;
    sVar1 = 4;
    do {
      if ((char)unaff_D7w == puVar2[0x1a]) {
        DAT_00fff0d4 = '\x04' - (char)sVar1;
                    /* WARNING: Subroutine does not return */
        FUN_0000c0da();
      }
      puVar2 = puVar2 + 1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    puVar2 = &DAT_00fffbbe;
    sVar1 = 4;
    while (-1 < (char)puVar2[0x1a]) {
      puVar2 = puVar2 + 1;
      sVar1 = sVar1 + -1;
      if (sVar1 == -1) {
                    /* WARNING: Subroutine does not return */
        FUN_0000c0da();
      }
    }
    DAT_00fff0d4 = '\x04' - (char)sVar1;
    FUN_0001957e();
    FUN_00018a92();
    FUN_0001952c();
    if ((short)(unaff_D6w - extraout_D1w) < 0) {
                    /* WARNING: Subroutine does not return */
      FUN_0000c0da();
    }
    FUN_00019736();
    FUN_000195d8();
    FUN_00019504();
    FUN_0001882e();
  }
  return;
}


