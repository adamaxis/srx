// Ghidra headless export
// Entry: 0x00010260
// Name: prepare_menu_grid_then_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000101FE select_menu_value
//   0x00012C5A write_menu_repeated_grid_with_metadata
//
// Source behavior: submit the magic menu metadata grid first, then select
// FFF098 from the cursor's table word and choose FFF096 from record +0x9D
// only on the negative +0x56, same-spell path.


void prepare_menu_grid_then_service(void)

{
  write_menu_repeated_grid_with_metadata();
  select_menu_value();
  return;
}


