// Ghidra headless export
// Entry: 0x00013874
// Name: dispatch_entity_action_candidates
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001438C collect_entity_action_primary_candidates
//   0x00014446 collect_entity_action_candidates


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void dispatch_entity_action_candidates(void)

{
  DAT_00ffe175 = 0;
  if (_DAT_00ffdfe8 == 0) {
    if ((DAT_00ffdf88 & 0x3f) == 0xb) {
      collect_entity_action_primary_candidates();
    }
    else if ((DAT_00ffdf88 & 0x3f) == 0xf) {
      collect_entity_action_candidates();
    }
  }
  else {
    collect_entity_action_primary_candidates();
    collect_entity_action_candidates();
  }
  return;
}

