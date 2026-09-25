// Ghidra headless export
// Entry: 0x0001E69C
// Name: initialize_world_effect_selection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound (first record and value rolls)
//   0x0001E68C reset_world_effect_globals
// Indirect call:
//   F3DC6[FFFBED] selected world-effect initializer

/*
 * F3EBC is a row-major table of eight six-byte records per context. The
 * first bounded roll chooses a record. Its word at +4, multiplied by the
 * source scale byte at FF018C and incremented, is the second roll bound;
 * the second roll contributes five units per result to FFFBF4. The selected
 * initializer is an intentional table boundary, not a ROM-address runtime
 * dependency in the native port.
 */

void initialize_world_effect_selection(void)
{
  ushort context;
  ushort first_roll;
  ushort record_offset;
  ushort second_bound;
  ushort second_roll;
  byte *record;

  DAT_00fff0b6 = 0;
  reset_world_effect_globals();
  context = DAT_00fff0ae;
  DAT_00fffbec = context;
  first_roll = sample_source_random_bound(8);
  record_offset = context * 0x30 + first_roll * 6;
  record = &DAT_000f3ebc + record_offset;
  DAT_00fffbec._1_1_ = record[0];
  DAT_00fffbf3 = record[1];
  DAT_00fff0c0 = *(ushort *)(record + 2);
  second_bound = record[4] * DAT_00ff018c + 1;
  second_roll = sample_source_random_bound(second_bound);
  DAT_00fffbf4 = *(ushort *)(record + 2) + second_roll * 5;
  dispatch_selected_world_effect_initializer(DAT_00fffbec._1_1_);
}
