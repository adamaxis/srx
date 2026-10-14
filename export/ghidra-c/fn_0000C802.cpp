// Ghidra headless export
// Entry: 0x0000C802
// Name: refresh_field_runner_state_and_assets
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 upload_field_asset_setup
//   0x000038FA run_entity_animation_triplet
//   0x0000391A handle_mode_action_event
//   0x0000C8B0 apply_byte_counter_delta_with_negative_floor
//   0x0001E514 clear_effect_slot_state
//   0x00059798 serialize_runner_record_population


void refresh_field_runner_state_and_assets(void)

{
  int extraout_A0;
  int extraout_A0_00;
  
  serialize_runner_record_population();
  if (((DAT_00ff0210 & 1) != 0) && ((DAT_00ff024c == '\0' || (DAT_00ff029c == '\0')))) {
    DAT_00fffc0c = 0xff;
    apply_byte_counter_delta_with_negative_floor();
    clear_effect_slot_state();
    if (extraout_A0 == DAT_00fffc04) {
      handle_mode_action_event();
    }
  }
  if (((DAT_00ff0310 & 1) != 0) && ((DAT_00ff034c == '\0' || (DAT_00ff039c == '\0')))) {
    DAT_00fffc0d = 0xff;
    apply_byte_counter_delta_with_negative_floor();
    clear_effect_slot_state();
    if (extraout_A0_00 == DAT_00fffc04) {
      handle_mode_action_event();
    }
  }
  upload_field_asset_setup();
  run_entity_animation_triplet();
  return;
}


