// Ghidra headless export
// Entry: 0x00013A92
// Name: FUN_00013a92
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013AE6 advance_entity_action_threshold
//   0x00013B12 FUN_00013b12
//   0x00013BBA FUN_00013bba
//   0x00013CD4 passes_entity_action_threshold


void FUN_00013a92(void)

{
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  bool bVar1;
  
  if (*(char *)(in_A0 + 0x9b) < '\x02') {
    bVar1 = false;
    if ((*(char *)(in_A0 + 0x56) < '\0') && (FUN_00013bba(), in_A0 = extraout_A0, bVar1)) {
      passes_entity_action_threshold();
      if (!bVar1) {
        return;
      }
      if (((-1 < *(char *)(extraout_A0_00 + 0x56)) && (*(char *)(extraout_A0_00 + 0x57) != '\0')) &&
         (*(char *)(extraout_A0_00 + 0x57) < '\r')) {
        return;
      }
      advance_entity_action_threshold();
      return;
    }
    bVar1 = *(byte *)(in_A0 + 0x9c) < 5;
    if ('\x04' < (char)*(byte *)(in_A0 + 0x9c)) {
      FUN_00013b12();
      if (bVar1) {
        find_first_entity_action_below_threshold();
        set_entity_action_state();
        return;
      }
      return;
    }
  }
  return;
}


