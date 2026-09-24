// Ghidra headless export
// Entry: 0x00059318
// Name: initialize_save_roster_buffer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000597EE project_live_runner_record_to_compact


void initialize_save_roster_buffer(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  
  sVar1 = 0x13;
  puVar2 = &DAT_00059972;
  puVar3 = &DAT_00fff1e8;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  project_live_runner_record_to_compact();
  sVar1 = 0xb;
  do {
    project_live_runner_record_to_compact();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00fffc0c = 0xff;
  DAT_00fffc0d = 0xff;
  return;
}


