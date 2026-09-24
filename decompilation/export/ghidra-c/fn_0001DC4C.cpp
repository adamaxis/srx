// Ghidra headless export
// Entry: 0x0001DC4C
// Name: FUN_0001dc4c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined2 * FUN_0001dc4c(void)

{
  char cVar1;
  undefined *in_D1;
  short unaff_D7w;
  short sVar2;
  int in_A0;
  undefined2 *puVar3;
  undefined2 *puVar4;
  
  if ((*(char *)(in_A0 + 0x56) < '\0') &&
     (((cVar1 = *(char *)(in_A0 + 0x57), cVar1 == '\f' || (cVar1 == '\x04')) || (cVar1 == '\v')))) {
    puVar3 = *(undefined2 **)(in_A0 + 0xa2);
    puVar4 = DAT_00fffc04;
    if (puVar3 != (undefined2 *)0x0) {
      *(byte *)((int)puVar3 + 0x11) = *(byte *)((int)puVar3 + 0x11) & 0xef;
      *(undefined4 *)(in_A0 + 0xa2) = 0;
      puVar4 = puVar3;
    }
    while( true ) {
      if (in_D1 == &UNK_000678cc) {
        in_D1 = (undefined *)0x60000;
      }
      else {
        puVar4 = puVar4 + 0x80;
      }
      puVar3 = (undefined2 *)CONCAT22((short)((uint)puVar3 >> 0x10),puVar4[0x1f]);
      if (2 < (short)puVar4[0x1f]) {
        puVar4 = &DAT_00ff0100;
      }
      if ((*(byte *)(puVar4 + 8) & 1) != 0) break;
      unaff_D7w = unaff_D7w + -1;
      if (unaff_D7w == -1) {
        return puVar3;
      }
    }
    *(byte *)((int)puVar4 + 0x11) = *(byte *)((int)puVar4 + 0x11) | 0x10;
    *(undefined2 **)(in_A0 + 0xa2) = puVar4;
    puVar4[0x53] = 0x8009;
    return puVar3;
  }
  sVar2 = 7;
  puVar3 = *(undefined2 **)(in_A0 + 0xa2);
  if (puVar3 != (undefined2 *)0x0) {
    *(byte *)((int)puVar3 + 0x11) = *(byte *)((int)puVar3 + 0x11) & 0xef;
    *(undefined4 *)(in_A0 + 0xa2) = 0;
    if ((undefined2 *)0xff09ff < puVar3) goto LAB_0001dcd4;
    puVar4 = puVar3;
    if (in_D1 == &UNK_000678cc) goto LAB_0001dcb8;
    puVar4 = puVar3 + 0x80;
    goto LAB_0001dcb8;
  }
  puVar4 = *(undefined2 **)(in_A0 + 0xac);
  puVar3 = puVar4;
  if (puVar4 == (undefined2 *)0x0) goto LAB_0001dcd4;
  puVar3 = (undefined2 *)CONCAT22((short)((uint)puVar4 >> 0x10),puVar4[0x1f]);
  while ((((*(byte *)(puVar4 + 8) & 1) == 0 || ((*(byte *)(puVar4 + 8) & 0x40) != 0)) ||
         (((*(byte *)(puVar4 + 8) & 0x10) != 0 ||
          (((*(byte *)(puVar4 + 0x58) & 0x80) != 0 || (puVar4[6] == 0))))))) {
    sVar2 = sVar2 + -1;
    puVar4 = puVar4 + 0x80;
    if (sVar2 == -1) {
      return puVar3;
    }
LAB_0001dcb8:
    puVar3 = puVar4;
    if ((short)puVar4[0x1f] < 3) {
      puVar3 = (undefined2 *)&DAT_00ff0400;
    }
    puVar4 = puVar3;
    if ((undefined2 *)0xff0aff < puVar3) {
LAB_0001dcd4:
      puVar4 = (undefined2 *)&DAT_00ff0400;
    }
  }
  *(undefined1 *)(in_A0 + 0x43) = 0;
  *(byte *)((int)puVar4 + 0x11) = *(byte *)((int)puVar4 + 0x11) | 0x10;
  *(undefined2 **)(in_A0 + 0xa2) = puVar4;
  *(int *)(puVar4 + 0x56) = in_A0;
  puVar4[0x53] = 0x8009;
  return puVar3;
}


