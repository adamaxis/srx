// Ghidra headless export
// Entry: 0x000106DA
// Name: prepare_grid_then_write
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001047E refresh_ready_entity_animations


void prepare_grid_then_write(void)

{
  refresh_ready_entity_animations();
  write_vdp_grid();
  return;
}


