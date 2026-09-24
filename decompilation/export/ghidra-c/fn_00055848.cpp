// Ghidra headless export
// Entry: 0x00055848
// Name: FUN_00055848
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A FUN_00000d3a
//   0x00000DBA FUN_00000dba
//   0x00055740 FUN_00055740
//   0x00055786 FUN_00055786
//   0x00055ED6 FUN_00055ed6
//   0x00055F20 FUN_00055f20


short FUN_00055848(void)

{
  byte bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  
  FUN_00055ed6();
  FUN_00055f20();
  FUN_00055740();
  FUN_00055786();
  FUN_00000dba();
  bVar1 = DAT_00fff0dd >> 1;
  sVar2 = FUN_00000d3a();
  sVar3 = (sVar2 + 1) - (ushort)bVar1;
  if (sVar3 != 0 && (short)(ushort)bVar1 <= (short)(sVar2 + 1)) {
    sVar2 = FUN_00000d3a();
    if ((short)(sVar2 + -1) < 0) {
      sVar4 = sVar3 * -0xc;
    }
    else {
      sVar4 = 0;
      if (sVar2 != 1) {
        sVar4 = sVar3 * 0xc;
      }
    }
    sVar2 = FUN_00000d3a();
    sVar3 = sVar2 + -1;
    if (((-1 < sVar3) && (sVar3 == 0 || sVar2 < 1)) && (sVar4 == 0)) {
      sVar3 = FUN_00000d3a();
    }
  }
  return sVar3;
}


