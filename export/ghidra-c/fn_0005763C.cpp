// Ghidra headless export
// Entry: 0x0005763C
// Name: select_menu_resource
// Decompile status: source-shaped annotation
//
// The native conversion is
// ghidra_conversion::select_menu_resource(). It preserves the source
// table-family choice (FFF0B6 == 6 selects the first 0x30-byte table; every
// other menu mode selects the second) and the saved-selection 0x10-byte
// record stride without exposing a ROM pointer to the native code.
// Calls recovered from direct call instructions:
//   (none)


int select_menu_resource(void)

{
  return ((DAT_00fff0b6 == 6) ? 0 : 0x30) +
         (uint)DAT_00fff0b8 * 0x10;
}

