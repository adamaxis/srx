// Ghidra headless export
// Entry: 0x0001A54E
// Name: initialize_effect_defaults
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)
//
// ROM source audit: writes FFE076=1, FFE078=0x1200, FFE07A=0x0D00, and
// FFE07C=0x1E.  It does not initialize a velocity, lifetime, or palette.
// The typed host state is WorldEffectDefaultState in
// ghidra_conversion/world/world_effect_controller_conversion.hpp.


void initialize_effect_defaults(void)

{
  DAT_00ffe076 = 1;
  DAT_00ffe078 = 0x1200;
  DAT_00ffe07a = 0xd00;
  DAT_00ffe07c = 0x1e;
  return;
}


