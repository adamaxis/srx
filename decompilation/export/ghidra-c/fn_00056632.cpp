// Ghidra headless export
// Entry: 0x00056632
// Name: scale_menu_hud_value_by_difficulty
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


// Direct disassembly clamps D4.w to the source minimum 2, multiplies D6.w by
// the resulting 100/80/60/40/20/10 percentage, and divides by 100. The typed
// host conversion is in ghidra_conversion/menu_hud_meter_conversion.
int scale_menu_hud_value_by_difficulty(void)

{
  int iVar1;
  short unaff_D4w;
  
  if ((short)(unaff_D4w + -2) < 0) {
    unaff_D4w = 2;
  }
  if ((ushort)(7U - unaff_D4w) == 0 || 7 < unaff_D4w) {
    iVar1 = 10;
  }
  else {
    iVar1 = (uint)(ushort)(7U - unaff_D4w) * 0x14;
  }
  return iVar1;
}

