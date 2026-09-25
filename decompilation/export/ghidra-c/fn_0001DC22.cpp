// Ghidra headless export
// Entry: 0x0001DC22
// Name: select_next_entity
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Removing unreachable block (ram,0x0001dcb4) */

undefined2 * select_next_entity(void)

{
  char cVar1;
  undefined2 *puVar2;
  undefined2 *in_D0;
  undefined *puVar3;
  short sVar4;
  undefined2 *puVar5;
  undefined2 *puVar6;
  
  puVar2 = DAT_00fffc04;
  sVar4 = 9;
  puVar5 = &DAT_00ff0100;
  while ((*(byte *)((int)puVar5 + 0x11) & 0x10) == 0) {
    puVar5 = puVar5 + 0x80;
    sVar4 = sVar4 + -1;
    if (sVar4 == -1) {
      return in_D0;
    }
  }
  puVar3 = &UNK_000678cc;
  if ((*(char *)(DAT_00fffc04 + 0x2b) < '\0') &&
     (((cVar1 = *(char *)((int)DAT_00fffc04 + 0x57), cVar1 == '\f' || (cVar1 == '\x04')) ||
      (cVar1 == '\v')))) {
    puVar5 = *(undefined2 **)(DAT_00fffc04 + 0x51);
    puVar6 = DAT_00fffc04;
    if (puVar5 != (undefined2 *)0x0) {
      *(byte *)((int)puVar5 + 0x11) = *(byte *)((int)puVar5 + 0x11) & 0xef;
      *(undefined4 *)(puVar2 + 0x51) = 0;
      puVar6 = puVar5;
    }
    while( true ) {
      if (puVar3 == &UNK_000678cc) {
        puVar3 = (undefined *)0x60000;
      }
      else {
        puVar6 = puVar6 + 0x80;
      }
      puVar5 = (undefined2 *)CONCAT22((short)((uint)puVar5 >> 0x10),puVar6[0x1f]);
      if (2 < (short)puVar6[0x1f]) {
        puVar6 = &DAT_00ff0100;
      }
      if ((*(byte *)(puVar6 + 8) & 1) != 0) break;
      sVar4 = sVar4 + -1;
      if (sVar4 == -1) {
        return puVar5;
      }
    }
    *(byte *)((int)puVar6 + 0x11) = *(byte *)((int)puVar6 + 0x11) | 0x10;
    *(undefined2 **)(puVar2 + 0x51) = puVar6;
    puVar6[0x53] = 0x8009;
    return puVar5;
  }
  sVar4 = 7;
  puVar5 = *(undefined2 **)(DAT_00fffc04 + 0x51);
  if (puVar5 != (undefined2 *)0x0) {
    *(byte *)((int)puVar5 + 0x11) = *(byte *)((int)puVar5 + 0x11) & 0xef;
    *(undefined4 *)(puVar2 + 0x51) = 0;
    puVar6 = puVar5;
    if ((undefined2 *)0xff09ff < puVar5) goto LAB_0001dcd4;
    goto LAB_0001dcb8;
  }
  puVar6 = *(undefined2 **)(DAT_00fffc04 + 0x56);
  puVar5 = puVar6;
  if (puVar6 == (undefined2 *)0x0) goto LAB_0001dcd4;
  puVar5 = (undefined2 *)CONCAT22((short)((uint)puVar6 >> 0x10),puVar6[0x1f]);
  while ((((*(byte *)(puVar6 + 8) & 1) == 0 || ((*(byte *)(puVar6 + 8) & 0x40) != 0)) ||
         (((*(byte *)(puVar6 + 8) & 0x10) != 0 ||
          (((*(byte *)(puVar6 + 0x58) & 0x80) != 0 || (puVar6[6] == 0))))))) {
    sVar4 = sVar4 + -1;
    puVar6 = puVar6 + 0x80;
    if (sVar4 == -1) {
      return puVar5;
    }
LAB_0001dcb8:
    puVar5 = puVar6;
    if ((short)puVar6[0x1f] < 3) {
      puVar5 = (undefined2 *)&DAT_00ff0400;
    }
    puVar6 = puVar5;
    if ((undefined2 *)0xff0aff < puVar5) {
LAB_0001dcd4:
      puVar6 = (undefined2 *)&DAT_00ff0400;
    }
  }
  *(undefined1 *)((int)puVar2 + 0x43) = 0;
  *(byte *)((int)puVar6 + 0x11) = *(byte *)((int)puVar6 + 0x11) | 0x10;
  *(undefined2 **)(puVar2 + 0x51) = puVar6;
  *(undefined2 **)(puVar6 + 0x56) = puVar2;
  puVar6[0x53] = 0x8009;
  return puVar5;
}


