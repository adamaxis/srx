// Ghidra headless export
// Entry: 0x0000A7B4
// Name: initialize_entity_runtime
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000A7E4 initialize_entity_slot_order


void initialize_entity_runtime(void)

{
  DAT_00fffc04 = &DAT_00ff0100;
  DAT_00ffdf2c = 0xff0000;
  DAT_00ffdf2a = 0;
  DAT_00ffdf32 = 0xff1480;
  DAT_00ffdf30 = 0;
  initialize_entity_slot_order();
  return;
}


