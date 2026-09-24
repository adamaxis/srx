// Ghidra headless export
// Entry: 0x0001E166
// Name: FUN_0001e166
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x0001E1FC forward_effect_sequence
//   0x0001E2E0 begin_world_activity_if_idle
//   0x00059986 select_world_effect_sequence


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e166(void)

{
  ushort uVar1;
  short sVar2;
  ushort extraout_D1w;
  undefined4 unaff_A6;
  
  if ((((_DAT_00ffdfe8 == 0) && ((*(byte *)(DAT_00fffc04 + 0x11) & 0x80) == 0)) &&
      (DAT_00ffe18b == '\0')) &&
     (((DAT_00ffe16f != '\0' || (_DAT_00ffdf84 == 0)) && (DAT_00ffe177 == '\0')))) {
    if ((DAT_00ffdf88 & 0x3ff) != 10) {
      return;
    }
    uVar1 = advance_source_prng();
    if (uVar1 < extraout_D1w) {
      if (DAT_00ffe16f != '\0') {
        forward_effect_sequence();
        return;
      }
      sVar2 = select_world_effect_sequence();
      if (sVar2 != 0) {
        _DAT_00ffdd2c = unaff_A6;
        begin_world_activity_if_idle();
        return;
      }
      FUN_0001e216();
      return;
    }
  }
  return;
}


