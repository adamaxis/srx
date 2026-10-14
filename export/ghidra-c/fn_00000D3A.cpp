// Ghidra headless export
// Entry: 0x00000D3A
// Name: sample_source_random_bound
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng


int sample_source_random_bound(void)

{
  uint uVar1;
  ushort extraout_D1w;
  
  uVar1 = advance_source_prng();
  return (int)(short)((uVar1 & 0xffff) * (uint)extraout_D1w >> 0x10);
}


