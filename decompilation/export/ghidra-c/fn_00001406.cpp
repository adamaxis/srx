// Ghidra headless export
// Entry: 0x00001406
// Name: prepare_title_resources
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000CEC FUN_00000cec
//   0x00000E34 reset_vdp_upload_state
//   0x00000F5C FUN_00000f5c
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x00001232 write_vdp_grid
//   0x0000131C step_palette_transition
//   0x00001368 copy_target_palette
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream


void prepare_title_resources(void)

{
  uint uVar1;
  int unaff_A6;
  uint *puVar2;
  uint *puVar3;
  
  step_palette_transition();
  FUN_00000cec();
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  reset_vdp_upload_state();
  copy_target_palette();
  DAT_00ffdd08 = *(undefined2 *)(unaff_A6 + 8);
  DAT_00ffdd0c = *(undefined2 *)(unaff_A6 + 10);
  DAT_00ffdd0a = *(undefined2 *)(unaff_A6 + 0xc);
  puVar2 = (uint *)(unaff_A6 + 0x10);
  DAT_00ffdd0e = *(undefined2 *)(unaff_A6 + 0xe);
  while( true ) {
    puVar3 = puVar2 + 1;
    if (*puVar2 == 0) break;
    if ((*puVar2 & 1) == 0) {
      puVar2 = puVar2 + 3;
      FUN_00000f5c();
    }
    else {
      puVar2 = puVar2 + 3;
      decode_and_upload_source_stream();
    }
  }
  while (uVar1 = *puVar3, uVar1 != 0) {
    puVar3 = (uint *)((int)puVar3 + 0xe);
    if ((uVar1 & 1) == 0) {
      write_vdp_grid();
    }
    else {
      render_hud_sprite_stream();
    }
  }
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  wait_for_vblank();
  return;
}


