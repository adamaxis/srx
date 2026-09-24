// Ghidra headless export
// Entry: 0x0001DBF0
// Name: FUN_0001dbf0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001dbf0(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  _DAT_00ffdf84 = 0;
  puVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    *(undefined4 *)(puVar2 + 0x51) = 0;
    *(undefined4 *)(puVar2 + 0x56) = 0;
    *(undefined1 *)((int)puVar2 + 0x43) = 0;
    *(byte *)(puVar2 + 0x58) = *(byte *)(puVar2 + 0x58) & 0xbf;
    *(byte *)((int)puVar2 + 0x11) = *(byte *)((int)puVar2 + 0x11) & 0xef;
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


