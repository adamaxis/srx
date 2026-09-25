// Ghidra headless export
// Entry: 0x00020634
// Name: select_mode_entry_resource
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::select_mode_entry_resource.
// The byte written to FFF0B4 is a host mode-resource profile. The subsequent
// resource service uses it to choose one of five host data profiles; it is
// not a request to reproduce the original pointer table at runtime.

void select_mode_entry_resource(void)
{
  if ((DAT_00fffbf8 == 0) && (DAT_00fffbf9 == 4)) {
    DAT_00fff0b4 = 2;
  } else if ((DAT_00fffbf8 == 0) && (DAT_00fffbf9 == 5)) {
    DAT_00fff0b4 = 1;
  } else if ((DAT_00fffbf8 == 3) && (DAT_00fffbf9 == 2)) {
    DAT_00fff0b4 = 1;
  } else if ((DAT_00fffbf8 == 3) && (DAT_00fffbf9 == 3)) {
    DAT_00fff0b4 = 2;
  } else if ((DAT_00fffbf8 == 3) && (DAT_00fffbf9 == 6)) {
    DAT_00fff0b4 = 0;
  } else if ((DAT_00fffbf8 == 1) && (DAT_00fffbf9 == 2)) {
    DAT_00fff0b4 = 4;
  } else if ((DAT_00fffbf8 == 2) && (DAT_00fffbf9 == 0)) {
    DAT_00fff0b4 = 0;
  } else if (DAT_00fffbf8 == 5) {
    DAT_00fff0b4 = 3;
  }
}
