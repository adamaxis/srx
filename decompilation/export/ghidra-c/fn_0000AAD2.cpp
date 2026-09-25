// Ghidra headless export
// Entry: 0x0000AAD2
// Name: dispatch_player_action_if_ready
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014226 convert_entity_routine_0x00014226


void dispatch_player_action_if_ready(void)

{
  if (((*(char *)(DAT_00fffc04 + 0x4e) == '\0') && (*(char *)(DAT_00fffc04 + 0x4f) != '\0')) &&
     (*(short *)(DAT_00fffc04 + 0xdc) < 0)) {
    convert_entity_routine_0x00014226();
    return;
  }
  return;
}


