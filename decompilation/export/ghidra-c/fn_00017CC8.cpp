// Ghidra headless export
// Entry: 0x00017CC8
// Name: FUN_00017cc8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001728 render_hud_sprite_stream
//   0x00017D3E advance_audio_service_tick


undefined8 FUN_00017cc8(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  DAT_00fff0de = DAT_00fff0de + -1;
  if (DAT_00fff0de < '\0') {
    DAT_00fff0de = '\x01';
    DAT_00fff0df = DAT_00fff0df + '\x01';
    if ('\x03' < DAT_00fff0df) {
      DAT_00fff0df = '\0';
    }
    render_hud_sprite_stream();
    if (DAT_00fff0df == '\0') {
      advance_audio_service_tick();
    }
  }
  return CONCAT44(in_D0,in_D1);
}


