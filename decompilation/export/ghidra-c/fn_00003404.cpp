// Ghidra headless export
// Entry: 0x00003404
// Name: FUN_00003404
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00003548 draw_hud_panel_triplet


void FUN_00003404(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  
  sVar1 = 2;
  do {
    sVar3 = 3;
    do {
      sVar2 = 3;
      do {
        write_vdp_word_block_with_source_wrap();
        sVar2 = sVar2 + -1;
      } while (sVar2 != -1);
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    sVar3 = 5;
    do {
      write_vdp_word_block_with_source_wrap();
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  draw_hud_panel_triplet();
  return;
}


