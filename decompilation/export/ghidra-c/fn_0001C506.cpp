// Ghidra headless export
// Entry: 0x0001C506
// Name: render_title_sprite_layers
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0001A570 build_title_sprites_twice
//
// ROM trace: 0x1C506 first builds A2 at FF1580(+0x0E/+0x10), then, when
// record bit 1 is set, selects one of the four direct tables pointed to by
// 0x1C5EC. The selected row is (signed FFE169 - 1), clamped only at 4, then
// masked with 3; the column is (FFDF88 & 3). A nonzero signed word is added
// to the base X and causes a second build. A4 is then built with the same
// overlay rule. The late 0x1A570 call is gated by FFE084 != 0 and
// (FFDF88 & 0xF) >= 8.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void render_title_sprite_layers(void)

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

