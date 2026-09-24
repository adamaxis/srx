// Ghidra headless export
// Entry: 0x0000E6F4
// Name: FUN_0000e6f4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00001232 upload_hud_tile_block


void FUN_0000e6f4(void)

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
  upload_hud_tile_block();
  return;
}


