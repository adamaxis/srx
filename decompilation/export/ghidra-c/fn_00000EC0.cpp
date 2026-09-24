// Ghidra headless export
// Entry: 0x00000EC0
// Name: FUN_00000ec0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00000ec0(void)

{
  char cVar1;
  undefined4 uVar2;
  word *pwVar3;
  
  uVar2 = 0xc000;
  pwVar3 = (word *)&DAT_00ffd87e;
  do {
    VDP_CTRL = (word)uVar2;
    VDP_CTRL = 0;
    VDP_DATA = *pwVar3;
    cVar1 = (char)uVar2 + '\x02';
    uVar2 = CONCAT31((int3)((uint)uVar2 >> 8),cVar1);
    pwVar3 = pwVar3 + 1;
  } while (-1 < cVar1);
  return;
}


