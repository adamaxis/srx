// Ghidra headless export
// Entry: 0x0001C506
// Name: FUN_0001c506
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0001A570 build_title_sprites_twice


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c506(void)

{
  ushort uVar1;
  int extraout_A1;
  int extraout_A1_00;
  undefined4 unaff_A2;
  undefined4 unaff_A4;
  int unaff_A6;
  
  build_title_sprite_records();
  if ((*(byte *)(extraout_A1 + 0x12) & 2) != 0) {
    uVar1 = (short)DAT_00ffe169 - 1;
    if (3 < (short)uVar1) {
      uVar1 = 3;
    }
    unaff_A6 = *(int *)((int)&PTR_DAT_0001c5ec + (int)(short)((uVar1 & 3) << 2));
    if (*(short *)(unaff_A6 + (short)((DAT_00ffdf88 & 3) << 1)) != 0) {
      build_title_sprite_records(unaff_A2);
    }
  }
  build_title_sprite_records();
  if (((*(byte *)(extraout_A1_00 + 0x12) & 2) != 0) &&
     (*(short *)(unaff_A6 + (short)((DAT_00ffdf88 & 3) << 1)) != 0)) {
    build_title_sprite_records(unaff_A4);
  }
  if ((_DAT_00ffe084 != 0) && (7 < (DAT_00ffdf88 & 0xf))) {
    build_title_sprites_twice();
  }
  return;
}


