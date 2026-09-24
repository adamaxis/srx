// Ghidra headless export
// Entry: 0x0001460E
// Name: apply_close_range_entity_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00015CA4 compute_diagonal_step_cost


void apply_close_range_entity_state(void)

{
  ushort uVar1;
  int extraout_A0;
  
  uVar1 = compute_diagonal_step_cost();
  if (uVar1 < 0x1e0) {
    *(byte *)(extraout_A0 + 0xb0) = *(byte *)(extraout_A0 + 0xb0) & 0x80;
    *(undefined1 *)(extraout_A0 + 0xb1) = 0;
    *(byte *)(extraout_A0 + 0xb0) = *(byte *)(extraout_A0 + 0xb0) | 4;
    *(undefined4 *)(extraout_A0 + 0x3a) = 0xb45c;
    return;
  }
  return;
}


