// Ghidra headless export
// Entry: 0x00005936
// Name: advance_entity_animation_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_entity_animation_effect(void)

{
  byte bVar1;
  int in_A0;
  
  if (*(short *)(in_A0 + 0xe) != 0xf) {
    bVar1 = *(byte *)(in_A0 + 6) - 1;
    *(byte *)(in_A0 + 6) = bVar1;
    if ((short)((ushort)bVar1 << 8) < 0) {
      *(undefined1 *)(in_A0 + 6) = 3;
      *(char *)(in_A0 + 7) = *(char *)(in_A0 + 7) + '\x01';
      if ('\x03' < *(char *)(in_A0 + 7)) {
        *(undefined1 *)(in_A0 + 7) = 0;
      }
    }
    build_title_sprite_records();
    return;
  }
  bVar1 = *(byte *)(in_A0 + 6) - 1;
  *(byte *)(in_A0 + 6) = bVar1;
  if ((short)((ushort)bVar1 << 8) < 0) {
    *(undefined1 *)(in_A0 + 6) = 3;
    *(char *)(in_A0 + 7) = *(char *)(in_A0 + 7) + '\x01';
    if ('\x03' < *(char *)(in_A0 + 7)) {
      *(undefined1 *)(in_A0 + 7) = 0;
    }
  }
  build_title_sprite_records
            (*(short *)(in_A0 + 10) + -0x2e,*(short *)(in_A0 + 0xc) + -0x20,
             *(undefined4 *)(&DAT_00005a46 + (short)(((short)*(char *)(in_A0 + 7) & 3U) << 2)));
  build_title_sprite_records();
  return;
}


