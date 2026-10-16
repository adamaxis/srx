// Ghidra headless export
// Entry: 0x0001CB6A
// Name: spawn_world_effect_record
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001CBF8 clear_world_effect_work
//   0x000530DE service_audio_if_enabled
//
// ROM trace: FFE010 selects the normal FF159E record or clears/selects the
// work record through 0x1CBF8. An active selected record returns before audio
// or field writes. Otherwise command 0x17 supplies D1 to +0x12, the record is
// seeded active with source variant 0, frame delay 0x14, phase delay 0, and
// lifetime 0x3C, then caller offsets D5/D6 are added to the source position
// and converted to fixed point. Destination deltas are divided by 0x3C for
// the two velocity words.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void spawn_world_effect_record(void)

{
  short sVar1;
  short sVar2;
  undefined2 extraout_D1w;
  short sVar3;
  short unaff_D5w;
  short unaff_D6w;
  int extraout_A0;
  undefined *extraout_A1;
  undefined *puVar4;
  int extraout_A1_00;
  
  if (_DAT_00ffe010 == 0) {
    puVar4 = &DAT_00ff159e;
  }
  else {
    clear_world_effect_work();
    puVar4 = extraout_A1;
  }
  if (puVar4[6] != '\0') {
    return;
  }
  service_audio_if_enabled();
  *(undefined2 *)(extraout_A1_00 + 0x12) = extraout_D1w;
  *(undefined1 *)(extraout_A1_00 + 6) = 0xff;
  *(undefined1 *)(extraout_A1_00 + 10) = 0;
  *(undefined1 *)(extraout_A1_00 + 9) = 0;
  *(undefined1 *)(extraout_A1_00 + 5) = 0;
  *(undefined1 *)(extraout_A1_00 + 4) = 0x14;
  sVar1 = *(short *)(extraout_A0 + 0xe) + unaff_D5w;
  sVar2 = *(short *)(extraout_A0 + 0x10) + unaff_D6w;
  *(short *)(extraout_A1_00 + 0x10) = sVar1 * 0x10;
  *(short *)(extraout_A1_00 + 2) = sVar2 * 0x10;
  sVar3 = DAT_00ff1598 * 0x10;
  *(short *)(extraout_A1_00 + 0xc) = (short)(DAT_00ff1596 * 0x10 + sVar1 * -0x10) / 0x3c;
  *(short *)(extraout_A1_00 + 0xe) = (short)(sVar3 + sVar2 * -0x10) / 0x3c;
  *(undefined1 *)(extraout_A1_00 + 0xb) = 0x3c;
  return;
}

