// Ghidra headless export
// Entry: 0x0000A7E4
// Name: initialize_entity_slot_order
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void initialize_entity_slot_order(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined2 *puVar3;
  
  puVar3 = &DAT_00ff0100;
  sVar1 = 0xf;
  puVar2 = &DAT_00ffde8a;
  do {
    *puVar2 = puVar3;
    puVar3 = puVar3 + 0x80;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


