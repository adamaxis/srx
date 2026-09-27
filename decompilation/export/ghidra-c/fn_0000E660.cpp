// Ghidra headless export
// Entry: 0x0000E660
// Name: advance_random_grid_effect
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00001232 write_vdp_grid


// Source behavior:
//   D1=2; random. If zero, D1=8; random and select the first primary table;
//   otherwise D1=4; random and select the second primary table. A third draw
//   uses bound 3 for the fixed CE126/CE12E/CE136 variant family on the zero
//   branch, or bound 4 for the indexed CE13E family on the nonzero branch.
//   Both branches submit a two-row grid with D1/D2 cleared and D7=1.
// The native conversion records this as RandomGridEffectPlan and leaves asset
// selection and host presentation to the callback; it does not emulate VDP
// registers or calculate ROM pointers at runtime.


