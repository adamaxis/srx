// Ghidra headless export
// Entry: 0x000556C6
// Name: FUN_000556c6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000556c6(void)

{
  byte bVar1;
  uint uVar2;
  short sVar3;
  int in_A0;
  
  bVar1 = *(byte *)(in_A0 + 0x57);
  uVar2 = 0;
  if ((-1 < (char)(bVar1 - 8)) && ((char)(bVar1 - 0xb) < '\0')) {
    uVar2 = (uint)(ushort)(bVar1 - 1);
  }
  if ((*(byte *)(in_A0 + 0x6e +
                (int)(short)((short)CONCAT31((int3)(uVar2 >> 8),*(undefined1 *)(in_A0 + 0x57)) + -1)
                ) & 1) != 0) {
    if ((*(ushort *)(in_A0 + 0x90) & 0x10) != 0) {
      return;
    }
    sVar3 = 7;
    do {
      if (*(char *)(in_A0 + 0x66 + (int)sVar3) == '\x16') {
        return;
      }
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
  }
  return;
}


