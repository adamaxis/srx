// Ghidra headless export
// Entry: 0x0001D3F4
// Name: expand_random_nibble_mask
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound

// The source table at 0x0F1C5E contains nine pointers. Each pointed-to bank
// begins with its selectable-count byte; 0x1D3F4 samples in that count-sized
// range, rotates the selected pattern through carry, and expands its eight
// bits to eight 0x0F/0x00 nibbles.

uint expand_random_nibble_mask(void)

{
  int iVar1;
  char cVar2;
  short in_D0w;
  short sVar4;
  uint uVar3;
  byte bVar5;
  
  iVar1 = *(int *)(&DAT_000f1c5e + (short)(in_D0w << 2));
  sVar4 = sample_source_random_bound();
  bVar5 = *(byte *)(iVar1 + 1 + (int)sVar4);
  uVar3 = 0;
  sVar4 = 7;
  do {
    uVar3 = uVar3 << 4;
    cVar2 = (char)bVar5 >> 7;
    bVar5 = bVar5 << 1 | -cVar2;
    if (-cVar2 != 0) {
      uVar3 = uVar3 | 0xf;
    }
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  return uVar3;
}


