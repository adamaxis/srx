// Ghidra headless export
// Entry: 0x0000313A
// Name: copy_terrain_layers
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_terrain_layers(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  sVar1 = 0xe0f;
  puVar2 = DAT_00ffddda;
  puVar3 = &DAT_00ff2700;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  sVar1 = 0xe0f;
  puVar2 = DAT_00ffdde2;
  puVar3 = &DAT_00ff4700;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}


