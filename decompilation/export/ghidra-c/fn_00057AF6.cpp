// Ghidra headless export
// Entry: 0x00057AF6
// Name: apply_runner_cost_surcharge
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00057B04 calculate_runner_cost_adjustment

void apply_runner_cost_surcharge(void)
{
  calculate_runner_cost_adjustment();
  D1 += D3;
  _DAT_00fff0be = D1;
}
