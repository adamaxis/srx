// Ghidra headless export
// Entry: 0x0001A16A
// Name: FUN_0001a16a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_title_text_source
//   0x00017C3C FUN_00017c3c
//   0x00018A92 FUN_00018a92
//   0x00055F44 FUN_00055f44


/* WARNING: Removing unreachable block (ram,0x0001a1d2) */
/* WARNING: Removing unreachable block (ram,0x0001a1da) */
/* WARNING: Removing unreachable block (ram,0x0001a20e) */
/* WARNING: Removing unreachable block (ram,0x0001a220) */
/* WARNING: Removing unreachable block (ram,0x0001a222) */
/* WARNING: Removing unreachable block (ram,0x0001a22e) */
/* WARNING: Removing unreachable block (ram,0x0001a230) */
/* WARNING: Removing unreachable block (ram,0x0001a270) */
/* WARNING: Removing unreachable block (ram,0x0001a278) */

undefined8 FUN_0001a16a(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  write_vdp_grid();
  write_vdp_grid();
  if (DAT_00fff0b0 < '\0') {
    FUN_00018a92();
    write_repeated_vdp_grid();
    FUN_00055f44();
    write_repeated_vdp_grid();
  }
  else {
    render_title_text_source();
  }
  return CONCAT44(in_D0,in_D1);
}


