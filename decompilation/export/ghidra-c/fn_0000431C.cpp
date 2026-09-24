// Ghidra headless export
// Entry: 0x0000431C
// Name: FUN_0000431c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005966E FUN_0005966e


void FUN_0000431c(void)

{
  undefined4 *puVar1;
  uint in_D0;
  short sVar2;
  undefined1 unaff_D7b;
  undefined4 *puVar3;
  undefined4 *puVar4;
  int extraout_A1;
  
  if ((char)in_D0 != (char)(DAT_00fffc0c + -1)) {
    DAT_00fffc0d = (char)in_D0 + '\x01';
    sVar2 = 0x3f;
    puVar3 = (undefined4 *)(&DAT_00072c1a + (in_D0 & 0xffff) * 0x100);
    puVar1 = (undefined4 *)&DAT_00ff0300;
    do {
      puVar4 = puVar1;
      *puVar4 = *puVar3;
      sVar2 = sVar2 + -1;
      puVar3 = puVar3 + 1;
      puVar1 = puVar4 + 1;
    } while (sVar2 != -1);
    *(undefined2 *)((int)puVar4 + -0xbe) = 2;
    *(undefined2 *)(puVar4 + -0x37) = 0x31;
    *(undefined2 *)((int)puVar4 + -0xd6) = 0x620;
  }
  FUN_0005966e();
  *(undefined1 *)(extraout_A1 + 0x8f) = unaff_D7b;
  return;
}


