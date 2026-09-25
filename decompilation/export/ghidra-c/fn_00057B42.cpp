// Ghidra headless export
// Entry: 0x00057B42
// Name: apply_large_field_action_cost_discount
// Decompile status: ok
//
// This is selected by callers after their computed cost reaches 0x10000. It
// uses the original cost >> 8 adjustment ten times, rather than the normal
// cost >> 5 runner adjustment.

void apply_large_field_action_cost_discount(void)
{
  const int margin = (int)(unsigned char)*(DAT_00fffc04 + 0x8c) -
                     (int)(unsigned char)(&DAT_001aac6e)[DAT_00fff0b6 * 2];
  if (margin > 0) {
    const int capped_margin = margin > 10 ? 10 : margin;
    const long adjustment = (D1 >> 8) * capped_margin;
    for (int iteration = 0; iteration < 10; ++iteration) {
      D1 -= adjustment;
    }
  }
  _DAT_00fff0be = D1;
}
