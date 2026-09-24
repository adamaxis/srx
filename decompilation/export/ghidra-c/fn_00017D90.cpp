// Ghidra headless export
// Entry: 0x00017D90
// Name: FUN_00017d90
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00017d90(void)

{
  char cVar1;
  char cVar2;
  char in_D1b;
  char *pcVar3;
  
  pcVar3 = (char *)(DAT_00fffc04 + 0x4c);
  cVar1 = *pcVar3;
  cVar2 = in_D1b + cVar1;
  *pcVar3 = cVar2;
  if (cVar2 == '\0' || SCARRY1(in_D1b,cVar1) != cVar2 < '\0') {
    *pcVar3 = '\0';
  }
  else if ('\n' < *pcVar3) {
    *pcVar3 = '\n';
  }
  FUN_00001232();
  FUN_0001792e();
  return;
}


