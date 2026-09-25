// Ghidra headless export
// Entry: 0x00013874
// Name: dispatch_entity_action_candidates
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001438C convert_entity_routine_0x0001438c
//   0x00014446 collect_entity_action_candidates


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void dispatch_entity_action_candidates(void)

{
  DAT_00ffe175 = 0;
  if (_DAT_00ffdfe8 == 0) {
    if ((DAT_00ffdf88 & 0x3f) == 0xb) {
      convert_entity_routine_0x0001438c();
    }
    else if ((DAT_00ffdf88 & 0x3f) == 0xf) {
      collect_entity_action_candidates();
    }
  }
  else {
    convert_entity_routine_0x0001438c();
    collect_entity_action_candidates();
  }
  return;
}


