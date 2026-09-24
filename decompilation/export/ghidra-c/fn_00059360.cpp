// Ghidra headless export
// Entry: 0x00059360
// Name: coordinate_save_load
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00059372 load_save_work_buffer
//   0x00059456 normalize_sram_save_slots


void coordinate_save_load(void)

{
  undefined1 in_CF;
  
  normalize_sram_save_slots();
  if (!(bool)in_CF) {
    load_save_work_buffer();
  }
  return;
}


