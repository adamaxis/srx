// Ghidra headless export
// Entry: 0x00000DBA
// Name: FUN_00000dba
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A FUN_00000d3a


undefined8 FUN_00000dba(void)

{
  undefined4 in_D0;
  short sVar1;
  undefined4 in_D1;
  short sVar2;
  short unaff_D4w;
  short unaff_D6w;
  short sVar3;
  
  sVar3 = unaff_D6w + -1;
  if (sVar3 < 0) {
    sVar3 = 0;
  }
  DAT_00fff0dd = '\0';
  do {
    sVar2 = 0;
    while( true ) {
      sVar1 = FUN_00000d3a();
      if ((short)(sVar1 + 1) != 6) break;
      sVar2 = sVar2 + 6;
    }
    if (-1 < (short)((sVar2 + sVar1 + 1) - unaff_D4w)) {
      DAT_00fff0dd = DAT_00fff0dd + '\x01';
    }
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  return CONCAT44(in_D0,in_D1);
}


