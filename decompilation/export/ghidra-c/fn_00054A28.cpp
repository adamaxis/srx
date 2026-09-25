// Ghidra headless export
// Entry: 0x00054A28
// Name: render_mode_status_summary
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::render_mode_status_summary.
// The source renders two encoded fields and then builds the grouped decimal
// glyphs used by the mode-action status presentation.
//
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text (twice)
//   0x0000C216 build_grouped_decimal_glyphs

void render_mode_status_summary(void)
{
  render_encoded_title_text();
  render_encoded_title_text();
  build_grouped_decimal_glyphs();
}
