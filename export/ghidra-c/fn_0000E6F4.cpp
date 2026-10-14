// Ghidra headless export
// Entry: 0x0000E6F4
// Name: service_mode_world_state
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00001232 write_vdp_grid


// Source behavior:
//   Four bounded draws are always consumed: 2, 2, then 3 and either 3 or 4.
//   With first draw zero, the second draw selects C13E2 (+4-byte variant
//   offset) or C13DC (+0); the third draw indexes the primary table and the
//   fourth selects one of the fixed three variants. D7=1 means two rows.
//   With first draw nonzero, the second selects C13D0 (+0) or C13D6 (+2);
//   the third draw indexes the primary table and the fourth indexes CE13E.
//   D7=0 means one row. D1 and D2 are cleared before the grid submission.
// The native conversion records these choices in ModeWorldGridPlan rather
// than reproducing Genesis address arithmetic or VDP writes.


