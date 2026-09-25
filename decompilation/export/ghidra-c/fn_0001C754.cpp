// Ghidra headless export
// Entry: 0x0001C754
// Name: convert_world_routine_0x0001c754
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019976 convert_world_routine_0x00019976
//   0x0001CAD4 update_interaction_state
//   0x0001CB20 convert_world_routine_0x0001cb20
//   0x000530DE service_audio_if_enabled


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_world_routine_0x0001c754(void)

{
  int extraout_A0;
  
  if (_DAT_00ffe084 != 0) {
    initialize_behavior_record();
    *(undefined **)(extraout_A0 + 8) = &DAT_0001c2ae;
    *(undefined **)(extraout_A0 + 4) = &DAT_0001c2a8;
    return;
  }
  if ((DAT_00ff1592 & 1) != 0) {
    return;
  }
  if (DAT_00ff1584 == &PTR_DAT_0001c36a) {
    if ((DAT_00ff158d == '\x03') && (DAT_00ff1583 == '\x03')) {
      convert_world_routine_0x0001cb20();
      return;
    }
    if (DAT_00ff158d != '\x05') {
      return;
    }
  }
  else if (DAT_00ff1588 == &PTR_DAT_0001c350) {
    if ((DAT_00ff158c == '\x03') && (DAT_00ff1582 == '\x03')) {
      update_interaction_state();
      return;
    }
    if (DAT_00ff158c != '\x05') {
      return;
    }
  }
  if (((DAT_00ffe076 < 3) &&
      ((((DAT_00ffdd22._0_1_ & 0x40) != 0 || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
       ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) && (_DAT_00ffe082 == 0)) {
    convert_world_routine_0x00019976();
    return;
  }
  return;
}

