// Ghidra headless export
// Entry: 0x00057B04
// Name: calculate_runner_cost_adjustment
// Decompile status: ok (Ghidra did not model the D3 output register)
//
// The native conversion returns this value directly. The original routine
// leaves it in D3 for 0x57A6E/0x57AF6, using the active runner's +0x8C value
// against the mode threshold table at 0x1AAC6E.

void calculate_runner_cost_adjustment(void)
{
  movea.l((long)DAT_00fffc04, A1);
  const int margin = (int)(unsigned char)*(A1 + 0x8c) -
                     (int)(unsigned char)(&DAT_001aac6e)[DAT_00fff0b6 * 2];
  if (margin <= 0) {
    D3 = 0;
    return;
  }

  const int capped_margin = margin > 10 ? 10 : margin;
  D3 = ((unsigned long)D1 >> 5) * capped_margin;
}
