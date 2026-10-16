// Ghidra headless export
// Entry: 0x00057A44
// Name: calculate_field_action_record_cost
// Decompile status: ok (the source helper continues into 0x57A6E)
//
// A0 points at the selected three-byte field-action record. Byte 0 chooses
// the 0xD4540 record's base rate at +0x22; byte 2 optionally scales that rate
// by quantity / 10. The resulting amount is passed through the mode and
// runner discounts implemented at 0x57A6E.

void calculate_field_action_record_cost(void)
{
  uint record_type = (uint)(unsigned char)*A0;
  const long record_base = 0x00d4540 + record_type * 0x30;
  D1 = (uint)(unsigned short)*(record_base + 0x22);

  const uint quantity = (uint)(unsigned char)*(A0 + 2);
  if (quantity != 0) {
    D1 = (D1 * quantity / 10) & 0xffff;
  }

  // Fall through to 0x57A6E: apply mode flags, then the runner adjustment.
  apply_field_action_cost_discounts();
}
