// Ghidra headless export
// Entry: 0x00019736
// Name: clear_ready_menu_slot_timer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void clear_ready_menu_slot_timer(void)

{
  if ((-1 < DAT_00fff10c) && ((&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] != '\x06')) {
    return;
  }
  (&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] = 0;
  write_vdp_grid();
  return;
}


