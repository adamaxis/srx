// Ghidra headless export
// Entry: 0x00002C6C
// Name: prepare_field_viewports
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001406 prepare_title_resources
//   0x00002C92 render_normal_field_viewport
//   0x00002CB2 render_alternate_field_viewport
//   0x00016AFA FUN_00016afa
//   0x000566C2 FUN_000566c2


void prepare_field_viewports(void)

{
  prepare_title_resources();
  FUN_000566c2();
  if (DAT_00ffdf40 == 2) {
    FUN_00016afa();
  }
  render_normal_field_viewport();
  render_alternate_field_viewport();
  return;
}


