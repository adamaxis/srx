// Ghidra headless export
// Entry: 0x00005622
// Name: clear_entity_runtime_fields
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void clear_entity_runtime_fields(void)

{
  short sVar1;
  byte in_D1b;
  undefined2 *puVar2;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    *(undefined1 *)(puVar2 + 0x6f) = 0;
    *(byte *)((int)puVar2 + 0xdf) = in_D1b & *(byte *)((int)puVar2 + 0xdf);
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


