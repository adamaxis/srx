// Ghidra headless export
// Entry: 0x00059456
// Name: FUN_00059456
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001502 FUN_00001502
//   0x0005961C FUN_0005961c


void FUN_00059456(void)

{
  bool bVar1;
  
  bVar1 = false;
  FUN_0005961c();
  if (bVar1) {
    bVar1 = false;
    FUN_0005961c();
    if (bVar1) {
      return;
    }
    FUN_00001502();
  }
  else {
    bVar1 = false;
    FUN_0005961c();
    if (bVar1) {
      FUN_00001502();
    }
  }
  return;
}


