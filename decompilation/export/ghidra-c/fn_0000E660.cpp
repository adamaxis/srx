// Ghidra headless export
// Entry: 0x0000E660
// Name: FUN_0000e660
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00001232 FUN_00001232


void FUN_0000e660(void)

{
  bool bVar1;
  
  bVar1 = false;
  sample_source_random_bound();
  if (bVar1) {
    sample_source_random_bound();
    sample_source_random_bound();
  }
  else {
    sample_source_random_bound();
    sample_source_random_bound();
  }
  FUN_00001232();
  return;
}


