// Ghidra headless export
// Entry: 0x0001088C
// Name: FUN_0001088c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 FUN_000011c0
//   0x00001232 upload_hud_tile_block
//   0x0000C0DA render_title_text_source


void FUN_0001088c(void)

{
  if ((char)(&DAT_000d4ec5)
            [(short)((ushort)*(byte *)(DAT_00fff10e +
                                      (short)(((short)CONCAT31((int3)((uint)DAT_00fff0d1 * 7 >> 8),
                                                               DAT_00fff0d0 +
                                                               (char)((uint)DAT_00fff0d1 * 7)) + 1)
                                             * 2)) * 0x30)] < '\0') {
    FUN_000011c0();
    return;
  }
  render_title_text_source();
  upload_hud_tile_block();
  return;
}


