// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001DC4C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 * FUN_0001dc4c(void)

{
  char cVar1;
  int in_D1;
  short unaff_D7w;
  short sVar2;
  int in_A0;
  undefined1 *puVar3;
  undefined1 *puVar4;
  
  if ((*(char *)(in_A0 + 0x56) < '\0') &&
     (((cVar1 = *(char *)(in_A0 + 0x57), cVar1 == '\f' || (cVar1 == '\x04')) || (cVar1 == '\v')))) {
    puVar3 = *(undefined1 **)(in_A0 + 0xa2);
    puVar4 = _DAT_00fffc04;
    if (puVar3 != (undefined1 *)0x0) {
      puVar3[0x11] = puVar3[0x11] & 0xef;
      *(undefined4 *)(in_A0 + 0xa2) = 0;
      puVar4 = puVar3;
    }
    while( true ) {
      if (in_D1 == 0x678cc) {
        in_D1 = 0x60000;
      }
      else {
        puVar4 = puVar4 + 0x100;
      }
      puVar3 = (undefined1 *)CONCAT22((short)((uint)puVar3 >> 0x10),*(short *)(puVar4 + 0x3e));
      if (2 < *(short *)(puVar4 + 0x3e)) {
        puVar4 = &DAT_00ff0100;
      }
      if ((puVar4[0x10] & 1) != 0) break;
      unaff_D7w = unaff_D7w + -1;
      if (unaff_D7w == -1) {
        return puVar3;
      }
    }
    puVar4[0x11] = puVar4[0x11] | 0x10;
    *(undefined1 **)(in_A0 + 0xa2) = puVar4;
    *(undefined2 *)(puVar4 + 0xa6) = 0x8009;
    return puVar3;
  }
  sVar2 = 7;
  puVar3 = *(undefined1 **)(in_A0 + 0xa2);
  if (puVar3 != (undefined1 *)0x0) {
    puVar3[0x11] = puVar3[0x11] & 0xef;
    *(undefined4 *)(in_A0 + 0xa2) = 0;
    if ((undefined1 *)0xff09ff < puVar3) goto LAB_0001dcd4;
    puVar4 = puVar3;
    if (in_D1 == 0x678cc) goto LAB_0001dcb8;
    puVar4 = puVar3 + 0x100;
    goto LAB_0001dcb8;
  }
  puVar4 = *(undefined1 **)(in_A0 + 0xac);
  puVar3 = puVar4;
  if (puVar4 == (undefined1 *)0x0) goto LAB_0001dcd4;
  puVar3 = (undefined1 *)CONCAT22((short)((uint)puVar4 >> 0x10),*(undefined2 *)(puVar4 + 0x3e));
  while ((((puVar4[0x10] & 1) == 0 || ((puVar4[0x10] & 0x40) != 0)) ||
         (((puVar4[0x10] & 0x10) != 0 ||
          (((puVar4[0xb0] & 0x80) != 0 || (*(short *)(puVar4 + 0xc) == 0))))))) {
    sVar2 = sVar2 + -1;
    puVar4 = puVar4 + 0x100;
    if (sVar2 == -1) {
      return puVar3;
    }
LAB_0001dcb8:
    puVar3 = puVar4;
    if (*(short *)(puVar4 + 0x3e) < 3) {
      puVar3 = (undefined1 *)0xff0400;
    }
    puVar4 = puVar3;
    if ((undefined1 *)0xff0aff < puVar3) {
LAB_0001dcd4:
      puVar4 = (undefined1 *)0xff0400;
    }
  }
  *(undefined1 *)(in_A0 + 0x43) = 0;
  puVar4[0x11] = puVar4[0x11] | 0x10;
  *(undefined1 **)(in_A0 + 0xa2) = puVar4;
  *(int *)(puVar4 + 0xac) = in_A0;
  *(undefined2 *)(puVar4 + 0xa6) = 0x8009;
  return puVar3;
}


