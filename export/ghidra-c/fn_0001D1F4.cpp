// Ghidra headless export
// Entry: 0x0001D1F4
// Name: advance_title_reveal_transfer_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00000E9C make_vdp_transfer_command
//   0x0001A01E publish_menu_selection_then_text_variant
//   0x0001D1DC transfer_world_word_block
//   0x0001D3F4 expand_random_nibble_mask


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// Native conversion: shadowrun_ghidra::advance_title_reveal_transfer_service
// represents the reveal writes as typed host records and keeps the world/menu
// handoff explicit; it does not carry VDP commands into the native runtime.
// The ROM calls expand_random_nibble_mask inside the 0x47-iteration loop, so
// each reveal longword receives a fresh mask-bank PRNG sample. It is not one
// mask reused for the entire frame.
void advance_title_reveal_transfer_service(void)

{
  ushort uVar2;
  uint uVar1;
  short sVar3;
  int in_A0;
  int extraout_A1;
  
  DAT_00ffe008 = DAT_00ffe008 + -1;
  if (-1 < DAT_00ffe008) {
    if ((DAT_00ffdf88 & 3) == 0) {
      transfer_world_word_block();
    }
    sVar3 = 0x46;
    do {
      uVar2 = sample_source_random_bound();
      make_vdp_transfer_command();
      uVar1 = expand_random_nibble_mask();
      Ram00c00000 = *(uint *)(extraout_A1 + (short)(uVar2 & 0xfffc)) & uVar1;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    return;
  }
  *(byte *)(in_A0 + 0x12) = *(byte *)(in_A0 + 0x12) & 0xef;
  transfer_world_word_block();
  if (DAT_00ffe16b != '\0') {
    DAT_00ffe0ae = 0;
    DAT_00ffe0b0 = 4;
    _DAT_00ffdf8c = 1;
    DAT_00ffdf76 = 0x70;
    DAT_00ffdf78 = 0xaf;
    DAT_00ffdf72 = 0xaf;
    DAT_00ffdf7a = &UNK_0001d428;
    DAT_00ffdf7e = &UNK_000da038;
    DAT_00ffdf74 = 0x811b;
    DAT_00ffdf70 = 0x51;
    DAT_00ffdf82 = 0;
    return;
  }
  publish_menu_selection_then_text_variant();
  return;
}

