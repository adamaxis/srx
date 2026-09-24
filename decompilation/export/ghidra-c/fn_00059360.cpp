// Ghidra headless export
// Entry: 0x00059360
// Name: FUN_00059360
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00059372 FUN_00059372
//   0x00059456 normalize_sram_save_slots


void FUN_00059360(void)

{
  undefined1 in_CF;
  
  normalize_sram_save_slots();
  if (!(bool)in_CF) {
    FUN_00059372();
  }
  return;
}


