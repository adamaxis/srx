// Ghidra headless export
// Entry: 0x00016E08
// Name: all_scene_record_slots_active
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void all_scene_record_slots_active(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  puVar2 = &DAT_00ff164c;
  sVar1 = 9;
  do {
    if ((*(byte *)(puVar2 + 8) & 1) == 0) {
      return;
    }
    puVar2 = puVar2 + 9;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


