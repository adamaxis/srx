// Ghidra headless export
// Entry: 0x0002679E
// Name: FUN_0002679e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000F98 write_vdp_word_block
//   0x00017CC8 advance_hud_sprite_audio_pulse
//   0x00017FDA advance_menu_random_glitch
//   0x0001C3FA FUN_0001c3fa
//   0x00024E20 FUN_00024e20
//   0x000268E6 FUN_000268e6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0002679e(void)

{
  byte bVar1;
  ushort uVar2;
  ushort uVar3;
  byte *pbVar4;
  byte *pbVar5;
  byte *pbVar6;
  
  DAT_00fff0e1 = 0xff;
  if (*(char *)((int)&DAT_00ff1100 + (int)(short)(DAT_00fff118 * 0x14)) == '\0') {
    pbVar5 = &DAT_0014c11e;
  }
  else {
    pbVar5 = &DAT_0014c186;
  }
  DAT_00ffe1aa = &DAT_00ff9700;
  DAT_00ffe1ae = &DAT_00ff8700;
  pbVar4 = &DAT_0014c1eb;
  pbVar6 = pbVar5 + 1;
  uVar3 = (ushort)*pbVar5;
  do {
    uVar2 = 2;
    pbVar5 = pbVar6;
    while( true ) {
      while( true ) {
        do {
          wait_for_vblank();
          latch_controller_events();
          advance_hud_sprite_audio_pulse();
          advance_menu_random_glitch();
          uVar2 = uVar2 - 1;
        } while (uVar2 != 0xffff);
        bVar1 = *pbVar5;
        if (-1 < (char)bVar1) break;
        if ((bVar1 >> 4 & 7) != 0) {
          DAT_00ffdd28 = DAT_00ffdd28 - (ushort)(bVar1 >> 4 & 7);
        }
        if ((*pbVar5 & 7) != 0) {
          DAT_00ffdd2a = DAT_00ffdd2a - (ushort)(*pbVar5 & 7);
        }
        uVar2 = 0;
        pbVar5 = pbVar5 + 1;
      }
      DAT_00ffe1a2 = (ushort)(bVar1 >> 4) + DAT_00ffe1a2;
      pbVar6 = pbVar5 + 1;
      if (*pbVar5 != 0) break;
      uVar2 = (ushort)*pbVar4;
      pbVar4 = pbVar4 + 1;
      pbVar5 = pbVar6;
    }
    DAT_00ffe1a4 = (*pbVar5 & 0xff0f) + DAT_00ffe1a4;
    DAT_00ffe1ae = &DAT_00ff8700;
    FUN_00024e20();
    write_vdp_word_block();
    DAT_00ffdd02 = 1;
    FUN_0001c3fa();
    FUN_000268e6();
    _DAT_00ffd980 = 1;
    DAT_00ffef14 = DAT_00ffef14 | 2;
    uVar3 = uVar3 - 1;
  } while (uVar3 != 0xffff);
  return;
}


