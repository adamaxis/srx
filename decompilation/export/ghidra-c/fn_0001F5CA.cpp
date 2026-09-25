// Ghidra headless export
// Entry: 0x0001F5CA
// Name: convert_world_routine_0x0001f5ca
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C64C convert_graphics_routine_0x0000c64c
//   0x0001EEDC dispatch_world_effect
//   0x0001F65A convert_world_routine_0x0001f65a
//   0x0001F830 convert_world_routine_0x0001f830
//   0x0001FA78 convert_world_routine_0x0001fa78
//   0x0001FD80 convert_world_routine_0x0001fd80
//   0x00020634 convert_world_routine_0x00020634
//   0x00053A78 run_indexed_title_services


void convert_world_routine_0x0001f5ca(void)

{
  if (((DAT_00ff0110 & 0x40) == 0) && ((DAT_00ff0110 & 0x10) == 0)) {
    convert_world_routine_0x00020634();
    DAT_00fff10c = 0;
    DAT_00fff0cc = '\0';
    dispatch_world_effect();
    if ((-1 < DAT_00fff0cc) && (run_indexed_title_services(), -1 < DAT_00fff0cc)) {
      convert_world_routine_0x0001f65a();
      convert_world_routine_0x0001f830();
      convert_world_routine_0x0001fa78();
      convert_world_routine_0x0001fd80();
    }
    DAT_00fff0cc = 0;
    return;
  }
  convert_graphics_routine_0x0000c64c();
  return;
}


