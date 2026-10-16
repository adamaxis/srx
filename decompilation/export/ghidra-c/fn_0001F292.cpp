// Ghidra headless export
// Entry: 0x0001F292
// Name: prepare_mode_exit_resources
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0001F32E prepare_optional_mode_resource
//   0x0001F34A prepare_mode_resource_variant_b
//   0x0001F38A prepare_mode_resource_variant_a
//   0x000566E2 refresh_mode_resource_metadata
//   0x000566F6 refresh_mode_resource_state


void prepare_mode_exit_resources(void)

{
  if (DAT_00fff0b5 != '\0') {
    prepare_optional_mode_resource();
  }
  refresh_mode_resource_metadata();
  refresh_mode_resource_state();
  decode_and_upload_source_stream();
  if (DAT_00fff0af < '\0') {
    if (DAT_00fff0ae < '\0') {
      DAT_00fff0ae = '\0';
    }
    prepare_mode_resource_variant_a();
    prepare_mode_resource_variant_b();
    DAT_00fff0af = '\0';
  }
  else {
    render_hud_sprite_stream();
  }
  return;
}


