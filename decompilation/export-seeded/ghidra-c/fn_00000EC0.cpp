// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000EC0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00000ec0(void)

{
  char cVar1;
  undefined2 *puVar2;
  
  cVar1 = '\0';
  puVar2 = (undefined2 *)&DAT_00ffd87e;
  do {
    _DAT_00c00004 = 0;
    _DAT_00c00000 = *puVar2;
    cVar1 = cVar1 + '\x02';
    puVar2 = puVar2 + 1;
  } while (-1 < cVar1);
  return;
}


