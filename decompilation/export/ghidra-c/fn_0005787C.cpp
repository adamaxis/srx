// Ghidra headless export
// Entry: 0x0005787C
// Name: calculate_field_action_quantity_cost
// Decompile status: ok
//
// This is a field-menu cost routine, not intro code. FFF0D0 is the selected
// cost category and FFF0D6 is the requested quantity. The result is published
// in FFF0BE for the caller to render or charge.

void calculate_field_action_quantity_cost(void)
{
  const uint quantity = (uint)DAT_00fff0d6;
  uint cost;

  if (DAT_00fff0d0 < '\x01') {
    // Category 0: quantity cubed, then multiplied by 25.
    cost = quantity * quantity * quantity * 0x19;
  }
  else {
    // Categories 1 and 2 use 3 * quantity^2 * 100; category 3 and later use
    // 2 * quantity^2 * 100.
    const uint category_rate = DAT_00fff0d0 < '\x03' ? 3 : 2;
    cost = quantity * quantity * category_rate * 100;
  }
  _DAT_00fff0be = cost;
}
