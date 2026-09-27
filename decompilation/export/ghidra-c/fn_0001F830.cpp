// Ghidra headless export
// Entry: 0x0001F830
// Name: prepare_field_mode_exit
// Decompile status: source-shaped annotation
//
// The native conversion keeps this routine's certain state policy in
// ghidra_conversion::prepare_field_mode_exit. Prompt/resource selection is
// still an explicit mode-exit service boundary.
//
// Calls recovered from direct call instructions:
//   0x0000C928 run_hospital_options_menu
//   0x0001F246 run_mode_exit_services
//   0x0002196C mark_effect_bit

void prepare_field_mode_exit(void)
{
  if (DAT_00fffbf8 == 0 && DAT_00fffbf9 == 0) {
    DAT_00fff0b6 = 0x0B;

    if (DAT_00fff0d5 == 0 && (DAT_00fffc30 & (1UL << 3)) == 0 &&
        (DAT_00fffc1e & (1UL << 23)) != 0 &&
        (DAT_00fffc1e & (1UL << 24)) != 0) {
      if ((DAT_00fffc1e & (1UL << 25)) == 0) {
        DAT_00fffc34 = (DAT_00fffc34 & ~(1UL << 18)) | (1UL << 19);
        DAT_00ffdd64 = 0x19;
        mark_effect_bit();
      }
      else if ((DAT_00fffc1e & (1UL << 29)) == 0 &&
               (DAT_00fffc22 & 1) != 0) {
        DAT_00fffc22 &= ~1UL;
        DAT_00fffc2a |= 0x0200;
        DAT_00fffc34 &= ~(1UL << 20);
        DAT_00fffc38 &= ~((1UL << 2) | (1UL << 3) | (1UL << 11));
        DAT_00fffc1e |= (1UL << 29);
      }
    }
  }
  else if (DAT_00fffbf8 == 2 && DAT_00fffbf9 == 1) {
    DAT_00fff0b6 = 8;
  }
  else if (DAT_00fffbf8 == 1 && DAT_00fffbf9 == 6) {
    DAT_00fff0b6 = 9;
  }
  else if (DAT_00fffbf8 == 4 && DAT_00fffbf9 == 4) {
    DAT_00fff0b6 = 10;
  }
  else {
    return;
  }

  DAT_00fff0d5 = 0;
  run_mode_exit_services();
  if (DAT_00fff0d5 != 0) {
    DAT_00fff0d5 = 0;
    run_hospital_options_menu();
  }
}
