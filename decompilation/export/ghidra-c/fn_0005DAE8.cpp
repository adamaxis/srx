// Ghidra headless export
// Entry: 0x0005DAE8
// Name: clear_pre_title_resource_workspace
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005DAFC write_pre_title_resource_stream


void clear_pre_title_resource_workspace(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  sVar1 = 0x1bf;
  puVar2 = (undefined2 *)&DAT_00ffb740;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  write_pre_title_resource_stream();
  return;
}


