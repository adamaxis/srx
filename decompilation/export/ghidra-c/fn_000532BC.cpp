// Ghidra headless export
// Entry: 0x000532BC
// Name: FUN_000532bc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c
//   0x00000FFE FUN_00000ffe
//   0x00001232 FUN_00001232
//   0x00001358 copy_source_word_block
//   0x0000C0DA FUN_0000c0da
//   0x00053200 FUN_00053200
//   0x0005322C FUN_0005322c
//   0x000533EE FUN_000533ee


void FUN_000532bc(void)

{
  int iVar1;
  short sVar2;
  short sVar3;
  
  VDP_CTRL = 0x8134;
  FUN_00000ffe();
  copy_source_word_block();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_00000f5c();
  FUN_00001232();
  FUN_0000c0da();
  FUN_000533ee();
  FUN_0005322c();
  FUN_00053200();
  iVar1 = 0xc190;
  sVar3 = 9;
  do {
    sVar2 = 0xd;
    do {
      FUN_00001232(iVar1);
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    iVar1 = iVar1 + 0x100;
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  VDP_CTRL = 0x8174;
  return;
}


