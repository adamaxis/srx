// Ghidra headless export
// Entry: 0x0000C802
// Name: FUN_0000c802
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 upload_field_asset_setup
//   0x000038FA run_entity_animation_triplet
//   0x0000391A FUN_0000391a
//   0x0000C8B0 apply_byte_counter_delta_with_negative_floor
//   0x0001E514 FUN_0001e514
//   0x00059798 FUN_00059798


void FUN_0000c802(void)

{
  int extraout_A0;
  int extraout_A0_00;
  
  FUN_00059798();
  if (((DAT_00ff0210 & 1) != 0) && ((DAT_00ff024c == '\0' || (DAT_00ff029c == '\0')))) {
    DAT_00fffc0c = 0xff;
    apply_byte_counter_delta_with_negative_floor();
    FUN_0001e514();
    if (extraout_A0 == DAT_00fffc04) {
      FUN_0000391a();
    }
  }
  if (((DAT_00ff0310 & 1) != 0) && ((DAT_00ff034c == '\0' || (DAT_00ff039c == '\0')))) {
    DAT_00fffc0d = 0xff;
    apply_byte_counter_delta_with_negative_floor();
    FUN_0001e514();
    if (extraout_A0_00 == DAT_00fffc04) {
      FUN_0000391a();
    }
  }
  upload_field_asset_setup();
  run_entity_animation_triplet();
  return;
}


