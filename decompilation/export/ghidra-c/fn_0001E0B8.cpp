// Ghidra headless export
// Entry: 0x0001E0B8
// Name: promote_entity_effect_flags
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void promote_entity_effect_flags(void)

{
  short sVar1;
  undefined *puVar2;
  
  puVar2 = &DAT_00ff0400;
  sVar1 = 6;
  do {
    if ((puVar2[0x9e] & 2) != 0) {
      puVar2[0x9e] = puVar2[0x9e] | 4;
    }
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


