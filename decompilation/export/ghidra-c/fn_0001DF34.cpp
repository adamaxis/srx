// Ghidra headless export
// Entry: 0x0001DF34
// Name: resolve_nearby_world_effects
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000133A6 convert_menu_routine_0x000133a6
//   0x0001E514 clear_effect_slot_state
//   0x000530DE service_audio_if_enabled

/*
 * The three callers pass one projected entity position at a time. This leaf
 * scans ten 0x12-byte effect records at FF164C. A record is eligible only
 * when its active bit is set and both coordinates are strictly within 0x100
 * of the probe. 0x133A resolves the interaction code and returns the next
 * probe coordinates; non-negative records are then consumed, audio command
 * 0x24 is serviced, and interaction code 0x65 clears the referenced slot.
 */

void resolve_nearby_world_effects(void)
{
  ushort probe_x;
  ushort probe_y;
  short index;
  ushort *record;

  probe_x = in_D0w;
  probe_y = in_D1w;
  record = &DAT_00ff164c;
  for (index = 0; index < 10; ++index, record += 9) {
    if (((*(byte *)(record + 4) & 1) == 0) ||
        ((ushort)(record[0] - 0x100U) >= probe_x) ||
        (probe_x >= (ushort)(record[0] + 0x100U)) ||
        ((ushort)(record[2] - 0x100U) >= probe_y) ||
        (probe_y >= (ushort)(record[2] + 0x100U))) {
      continue;
    }

    const bool negative_marker = (short)record[1] < 0;
    probe_x = convert_menu_routine_0x000133a6(record[1]);
    probe_y = extraout_D1w;
    if (negative_marker) {
      continue;
    }
    service_audio_if_enabled(0x24);
    *(byte *)(record + 4) &= 0xfe;
    if (record[1] != 0x65) {
      continue;
    }
    clear_effect_slot_state((ushort)(record[3] + 0xff00U));
  }
}
