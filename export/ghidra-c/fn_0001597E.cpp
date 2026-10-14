// Ghidra headless export
// Entry: 0x0001597E
// Name: scan_entity_effect_targets
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00015CC0 compute_diagonal_step_cost


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void scan_entity_effect_targets(void)

{
  bool bVar1;
  char cVar2;
  ushort unaff_D3w;
  short sVar3;
  int in_A0;
  undefined2 *puVar4;
  undefined2 *extraout_A0;
  int in_A1;
  
  bVar1 = true;
  cVar2 = *(char *)(in_A0 + 0x57);
  if ((((cVar2 != '\x03') && (cVar2 != '\n')) && (cVar2 != '\x05')) &&
     ((bVar1 = false, cVar2 != '\x06' && (cVar2 != '\r')))) {
    return;
  }
  _DAT_00ffdfea = 0xffff;
  DAT_00ffdfec = 0;
  DAT_00ffdfee = *(undefined2 *)(in_A1 + 10);
  DAT_00ffdff0 = *(undefined2 *)(in_A1 + 0xc);
  puVar4 = &DAT_00ff0100;
  sVar3 = 9;
  do {
    if ((((*(byte *)(puVar4 + 8) & 1) != 0) && ((*(byte *)(puVar4 + 8) & 0x40) == 0)) &&
       (cVar2 = compute_diagonal_step_cost(), puVar4 = extraout_A0, unaff_D3w < 0x500)) {
      if (cVar2 == '\r') {
        *(byte *)((int)extraout_A0 + 0xdf) = *(byte *)((int)extraout_A0 + 0xdf) | 2;
        *(int *)(extraout_A0 + 0x6c) = in_A0;
      }
      else if (cVar2 == '\x06') {
        *(byte *)((int)extraout_A0 + 0xdf) = *(byte *)((int)extraout_A0 + 0xdf) | 1;
        *(int *)(extraout_A0 + 0x6c) = in_A0;
      }
      *(char *)(extraout_A0 + 0x6f) = cVar2;
      if (!bVar1) {
        bVar1 = true;
        *(char *)(extraout_A0 + 0x59) = cVar2;
        DAT_00ffdffa = extraout_A0;
      }
    }
    puVar4 = puVar4 + 0x80;
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  return;
}


