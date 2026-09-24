// Ghidra headless export
// Entry: 0x00059986
// Name: FUN_00059986
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound


undefined1 FUN_00059986(void)

{
  short sVar1;
  undefined3 uVar2;
  undefined6 uVar3;
  
  sVar1 = sample_source_random_bound();
  uVar3 = sample_source_random_bound();
  uVar2 = (undefined3)((uint6)uVar3 >> 8);
  return (&DAT_001d70e4)
         [(short)(sVar1 + (short)((uint6)uVar3 >> 0x20) +
                 (short)CONCAT31(uVar2,(&DAT_00003312)[(short)CONCAT31(uVar2,DAT_00fffbf8)]) * 9)];
}


