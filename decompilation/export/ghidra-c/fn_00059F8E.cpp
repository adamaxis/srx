// Ghidra headless export
// Entry: 0x00059F8E
// Name: advance_title_intro_overlay
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F1C write_transfer_header
//   0x00001728 render_hud_sprite_stream
//   0x000530DE service_audio_if_enabled
//   0x0005B32C FUN_0005b32c


void advance_title_intro_overlay(void)

{
  ushort uVar1;
  
  uVar1 = DAT_00ffdf88 - DAT_00fff11e;
  if (uVar1 < 0x169) {
    if (uVar1 == 0x168) {
      service_audio_if_enabled();
      DAT_00ffdd08 = 0;
      write_transfer_header();
      FUN_0005b32c();
      render_hud_sprite_stream();
    }
    else {
      if (0x11f < uVar1) {
        DAT_00fff124 = DAT_00fff124 + 0xaa;
        DAT_00ffdd08 = DAT_00fff124 >> 8;
        DAT_00ffef14 = DAT_00ffef14 | 4;
      }
      if (DAT_00fff122 <= DAT_00ffdf88) {
        DAT_00fff122 = DAT_00ffdf88 +
                       *(short *)((int)&PTR_DAT_0005a084 + (int)(short)(DAT_00fff120 * 6));
        render_hud_sprite_stream();
        DAT_00fff120 = DAT_00fff120 + 1;
        if (5 < DAT_00fff120) {
          DAT_00fff120 = 0;
        }
      }
    }
  }
  return;
}


