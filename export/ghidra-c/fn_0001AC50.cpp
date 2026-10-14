// Ghidra headless export
// Entry: 0x0001AC50
// Name: arm_effect_after_threshold
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)
//
// ROM source audit: the 0x7140 BCC gate publishes FFE084=1 and clears
// FFE086.  These fields are separate from 0x1A54E's defaults; the typed host
// state is WorldEffectOverlayState in the world-effect overlay module.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void arm_effect_after_threshold(void)

{
  ushort in_D0w;
  
  if (in_D0w < 0x7140) {
    return;
  }
  _DAT_00ffe084 = 1;
  DAT_00ffe086 = 0;
  return;
}


