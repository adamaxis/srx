// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001597E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001597e(void)

{
  bool bVar1;
  char cVar2;
  ushort unaff_D3w;
  short sVar3;
  int in_A0;
  undefined1 *puVar4;
  undefined1 *extraout_A0;
  int in_A1;
  
  bVar1 = true;
  cVar2 = *(char *)(in_A0 + 0x57);
  if ((((cVar2 != '\x03') && (cVar2 != '\n')) && (cVar2 != '\x05')) &&
     ((bVar1 = false, cVar2 != '\x06' && (cVar2 != '\r')))) {
    return;
  }
  _DAT_00ffdfea = 0xffff;
  _DAT_00ffdfec = 0;
  _DAT_00ffdfee = *(undefined2 *)(in_A1 + 10);
  _DAT_00ffdff0 = *(undefined2 *)(in_A1 + 0xc);
  puVar4 = &DAT_00ff0100;
  sVar3 = 9;
  do {
    if ((((puVar4[0x10] & 1) != 0) && ((puVar4[0x10] & 0x40) == 0)) &&
       (cVar2 = FUN_00015cc0(), puVar4 = extraout_A0, unaff_D3w < 0x500)) {
      if (cVar2 == '\r') {
        extraout_A0[0xdf] = extraout_A0[0xdf] | 2;
        *(int *)(extraout_A0 + 0xd8) = in_A0;
      }
      else if (cVar2 == '\x06') {
        extraout_A0[0xdf] = extraout_A0[0xdf] | 1;
        *(int *)(extraout_A0 + 0xd8) = in_A0;
      }
      extraout_A0[0xde] = cVar2;
      if (!bVar1) {
        bVar1 = true;
        extraout_A0[0xb2] = cVar2;
        _DAT_00ffdffa = extraout_A0;
      }
    }
    puVar4 = puVar4 + 0x100;
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  return;
}


