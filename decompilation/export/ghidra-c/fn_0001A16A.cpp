// Ghidra headless export
// Entry: 0x0001A16A
// Name: render_matrix_runner_detail_panel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00017C3C read_menu_entry_profile_eligibility
//   0x00018A92 matrix_program_index
//   0x00055F44 calculate_runner_action_delay
// Direct disassembly is authoritative for the branches removed by Ghidra's
// unreachable-block pass; the typed host conversion is in matrix_loop.


/* WARNING: Removing unreachable block (ram,0x0001a1d2) */
/* WARNING: Removing unreachable block (ram,0x0001a1da) */
/* WARNING: Removing unreachable block (ram,0x0001a20e) */
/* WARNING: Removing unreachable block (ram,0x0001a220) */
/* WARNING: Removing unreachable block (ram,0x0001a222) */
/* WARNING: Removing unreachable block (ram,0x0001a22e) */
/* WARNING: Removing unreachable block (ram,0x0001a230) */
/* WARNING: Removing unreachable block (ram,0x0001a270) */
/* WARNING: Removing unreachable block (ram,0x0001a278) */

undefined8 render_matrix_runner_detail_panel(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  write_vdp_grid();
  write_vdp_grid();
  if (DAT_00fff0b0 < '\0') {
    convert_world_routine_0x00018a92();
    write_repeated_vdp_grid();
    calculate_runner_action_delay();
    write_repeated_vdp_grid();
  }
  else {
    render_encoded_title_text();
  }
  return CONCAT44(in_D0,in_D1);
}

