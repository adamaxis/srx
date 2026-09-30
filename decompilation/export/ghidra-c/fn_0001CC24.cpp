// Ghidra headless export
// Entry: 0x0001CC24
// Name: advance_persistent_world_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019EA6 run_menu_selection_variant
//   0x0001D048 advance_world_effect_record
//   0x0001D0DE resolve_world_effect_sprite
//
// ROM trace: the six-slot ring index is record byte +0x08, not an unrelated
// controller variable. It advances only when FFDF88 & 3 == 0, wraps at six,
// stores the current resolved stream/coordinates into that slot, then walks
// seven wrapped slots beginning at the following entry with 0x6000 builds.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_persistent_world_effect(void)

{
  byte bVar2;
  ushort uVar1;
  undefined2 extraout_D1w;
  undefined2 unaff_D2w;
  undefined2 uVar3;
  short sVar4;
  int extraout_A0;
  int extraout_A0_00;
  int iVar5;
  short *psVar6;
  undefined4 unaff_A2;
  
  if (DAT_00ff15b8 != '\0') {
    advance_world_effect_record();
    resolve_world_effect_sprite();
    bVar2 = *(byte *)(extraout_A0 + 8);
    if ((DAT_00ffdf88 & 3) == 0) {
      bVar2 = bVar2 + 1;
    }
    if (5 < bVar2) {
      bVar2 = 0;
    }
    *(byte *)(extraout_A0 + 8) = bVar2;
    uVar1 = (short)(char)bVar2 << 3;
    iVar5 = (int)(short)uVar1;
    *(undefined2 *)((int)&DAT_00ffe012 + iVar5) = extraout_D1w;
    *(undefined4 *)((int)&DAT_00ffe014 + iVar5) = unaff_A2;
    *(undefined2 *)((int)&DAT_00ffe018 + iVar5) = unaff_D2w;
    sVar4 = 6;
    uVar3 = 0x6000;
    psVar6 = (short *)((int)&DAT_00ffe01a + iVar5);
    iVar5 = extraout_A0;
    do {
      if (*psVar6 != 0) {
        *(short *)((int)register0x0000003c + -4) = sVar4;
        *(undefined2 *)((int)register0x0000003c + -8) = uVar3;
        *(ushort *)((int)register0x0000003c + -0xc) = uVar1;
        *(undefined4 *)((int)register0x0000003c + -0x10) = 0x1ccaa;
        build_title_sprite_records();
        uVar1 = *(ushort *)((int)register0x0000003c + -0xc);
        uVar3 = *(undefined2 *)((int)register0x0000003c + -10);
        sVar4 = *(short *)((int)register0x0000003c + -8);
        register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -6);
        iVar5 = extraout_A0_00;
      }
      uVar1 = uVar1 + 8;
      if (0x2f < uVar1) {
        uVar1 = 0;
      }
      psVar6 = (short *)((int)&DAT_00ffe012 + (int)(short)uVar1);
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    if (*(char *)(iVar5 + 6) == '\0') {
      _DAT_00ffe0d6 = 0;
      *(undefined4 *)((int)register0x0000003c + -4) = 0x1ccda;
      run_menu_selection_variant();
    }
    return;
  }
  return;
}

