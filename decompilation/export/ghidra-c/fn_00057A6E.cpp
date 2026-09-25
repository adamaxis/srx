// Ghidra headless export
// Entry: 0x00057A6E
// Name: apply_field_action_cost_discounts
// Decompile status: ok (source helper publishes FFF0BE)
// Calls recovered from direct call instructions:
//   0x00057B04 calculate_runner_cost_adjustment

void apply_field_action_cost_discounts(void)
{
  // D7 is the integer one-percent unit used by the mode-specific discounts.
  const uint percent_unit = D1 / 100;
  if (DAT_00fff0b6 == 4 && (DAT_00ff018f & 0x40) != 0) {
    D1 -= percent_unit * 10;
  }
  if (DAT_00fff0b6 == 5 && (DAT_00ff018f & 0x80) != 0) {
    D1 -= percent_unit * 25;
  }
  if (DAT_00fff0b6 == 2 && (DAT_00ff018f & 0x04) != 0) {
    D1 >>= 1;
  }
  if (DAT_00fff0b6 == 8 && (DAT_00ff018f & 0x02) != 0) {
    D1 >>= 1;
  }

  calculate_runner_cost_adjustment();
  D1 -= D3;
  _DAT_00fff0be = D1;
}
