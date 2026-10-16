// Ghidra headless export
// Entry: 0x00026972
// Name: refresh_title_sprite_records
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::refresh_title_sprite_records.
// The title-record builder is a host callback; this function only owns the
// source visibility/countdown/throttle policy.
//
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records

void refresh_title_sprite_records(void)
{
  if (DAT_00fff0d2 == 0) {
    return;
  }
  if (DAT_00ffe0ee != 0) {
    DAT_00ffe0f0 = DAT_00ffe0f0 - 1;
    if (DAT_00ffe0f0 < 0) {
      DAT_00ffe0ee = 0;
    } else if (1 < (DAT_00ffdf88 & 3)) {
      return;
    }
  }
  build_title_sprite_records();
}
