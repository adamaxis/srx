// Ghidra headless export
// Entry: 0x0000E6F4
// Name: service_mode_world_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00001232 write_vdp_grid


void service_mode_world_state(void)

{
  bool bVar1;
  
  bVar1 = false;
  sample_source_random_bound();
  if (bVar1) {
    sample_source_random_bound();
    sample_source_random_bound();
    sample_source_random_bound();
  }
  else {
    sample_source_random_bound();
    sample_source_random_bound();
    sample_source_random_bound();
  }
  write_vdp_grid();
  return;
}


