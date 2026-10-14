// Ghidra headless export
// Entry: 0x0001088C
// Name: render_magic_spell_availability
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text


void render_magic_spell_availability(void)

{
  if ((char)(&DAT_000d4ec5)
            [(short)((ushort)*(byte *)(DAT_00fff10e +
                                      (short)(((short)CONCAT31((int3)((uint)DAT_00fff0d1 * 7 >> 8),
                                                               DAT_00fff0d0 +
                                                               (char)((uint)DAT_00fff0d1 * 7)) + 1)
                                             * 2)) * 0x30)] < '\0') {
    write_repeated_vdp_grid();
    return;
  }
  render_encoded_title_text();
  write_vdp_grid();
  return;
}


