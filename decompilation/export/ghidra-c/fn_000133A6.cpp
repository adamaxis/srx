// Ghidra headless export
// Entry: 0x000133A6
// Name: FUN_000133a6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound


undefined8 FUN_000133a6(void)

{
  undefined4 in_D0;
  ushort uVar1;
  char cVar2;
  undefined4 in_D1;
  short sVar3;
  int iVar4;
  int extraout_A0;
  
  iVar4 = DAT_00fffc04;
  uVar1 = (ushort)in_D0;
  if ((short)uVar1 < 0) {
    if ((short)(uVar1 << 1) < 0) {
      *(char *)(DAT_00fffc04 + 0x4f) = (char)in_D0 + *(char *)(DAT_00fffc04 + 0x4f);
      if (-1 < (char)(*(char *)(iVar4 + 0x4f) + -0x15)) {
        *(undefined1 *)(iVar4 + 0x4f) = 0x14;
LAB_00013432:
        return CONCAT44(in_D0,in_D1);
      }
    }
    else {
      DAT_00fffbfe = (short)(uVar1 & 0xfff) + DAT_00fffbfe;
    }
  }
  else if (uVar1 == 100) {
    DAT_00fffc22 = DAT_00fffc22 | 0x10;
  }
  else {
    if (uVar1 != 0x65) {
      sVar3 = 7;
      do {
        if (*(char *)(iVar4 + 0x66) == '\0') {
          *(char *)(iVar4 + 0x66) = (char)in_D0 + '\x01';
          *(undefined1 *)(iVar4 + 0x6e) = 0;
          if (-1 < (short)(uVar1 - 0xc)) {
            cVar2 = sample_source_random_bound();
            *(char *)(extraout_A0 + 0x6e) = cVar2 + '\x02';
          }
          goto LAB_0001345e;
        }
        iVar4 = iVar4 + 1;
        sVar3 = sVar3 + -1;
      } while (sVar3 != -1);
      goto LAB_00013432;
    }
    DAT_00fffc22 = DAT_00fffc22 | 8;
  }
LAB_0001345e:
  return CONCAT44(in_D0,in_D1);
}


